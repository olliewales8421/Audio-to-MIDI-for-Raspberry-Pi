#include <alsa/asoundlib.h>
#include <iostream>
#include <vector>
#include <csignal>
#include <atomic>
#include <lo/lo.h>
#include <unistd.h>
#include <cmath>

#define SAMPLE_RATE 48000

struct LowPassFilter
{ 
    float prevOutput, prevInput = 0.0f;

    float filter (float input){
        float freq = 100;
        float x = tanf(M_PI * freq / (float)SAMPLE_RATE);

        float output = ((x * input) + (x * prevInput)) - ((x-1) * prevOutput);
        output /= (x+1);

        prevOutput = output;
        prevInput = input;

        return output;
    }
    float smoothing (float input){
        float alpha = 0.2f;
        return (alpha * input) + ((1.0f - alpha) * input);
    }
};

struct ZeroCrossingDetector
{
    float prevSample = 0.0f;
    int count = 0;
    float threshold = 0.0f;

    // checks for zero-crossing
    void process(float sample){
        if ((sample >= threshold && prevSample <  -threshold) ||
            (sample <  -threshold && prevSample >= threshold)) {
            count++;
        }
        prevSample = sample;
    }
    
    // resets zero-crossing count
    void reset(){
        count = 0;
    }
};

struct AmplitudeCalculator
{
    float sum = 0.0f;
    int count = 0;

    float threshold = 0.08f;
    float decayThreshold = 0.05f;
    float lastAmp = 0.0f;

    void add (float sample){
        sum += std::abs(sample);
        count++;
    }

    float average(){
        return sum / (float)count;
    }

    void reset(){
        sum = 0.0f;
        count = 0;
    }
};

int midiConversion (float freq){
    return (int)((12 * log2(freq/440.0)) + 69.0);
}

void sendNote(lo_address t, bool noteOn, int note, int velocity)
{		    
    if (noteOn == true){
        lo_message m = lo_message_new();
        lo_message_add_int32(m, 0x90); //0x is hexadecimal, 9 is note on standard and 8 is off, 0 is channel 1
        lo_message_add_int32(m, note);
        lo_message_add_int32(m, velocity);
        lo_send_message(t, "/midi/noteon", m);
        lo_message_free(m);
        std::cout << "Sent Note On\n";
    }
    else {
        lo_message m = lo_message_new();
        lo_message_add_int32(m, 0x80); //0x is hexadecimal, 9 is note on standard and 8 is off, 0 is channel 1
        lo_message_add_int32(m, note);
        lo_message_add_int32(m, velocity);
        lo_send_message(t, "/midi/noteoff", m);
        lo_message_free(m);
        std::cout << "Sent Note Off\n";
    }
}


std::atomic<bool> keepRunning(true);
void handleSigInt(int) { keepRunning = false; }

int main(int argc, char **argv){
    
    // Prints error if arguments are not met
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <ip> <port>\n";
        return 1;
    }

    const char* ip = argv[1];
    const char* port = argv[2];

    lo_address oscTarget = lo_address_new(ip, port);
    
    // declares struct objects
    ZeroCrossingDetector zeroCrossing;
    LowPassFilter LPF;
    AmplitudeCalculator ampCalc;
    
    int note, lastNote, velocity = 0;
    bool noteOff = true;

    // 
    std::signal(SIGINT, handleSigInt);

    // Audio interface hardware device
    const char* captureDevice  = "hw:2,0";
    const char* playbackDevice = "hw:2,0";

    snd_pcm_t *pcmIn, *pcmOut;
    snd_pcm_hw_params_t *paramsIn, *paramsOut;

    unsigned int sampleRate = SAMPLE_RATE;
    int channels = 2;    // stereo

    // Low latency settings
    snd_pcm_uframes_t periodSize = 1280;
    snd_pcm_uframes_t bufferSize = periodSize * 3;

    int pcmReturn;
    float print = 0;
    float value = 0;

    // --------------------------
    // SETUP CAPTURE
    // --------------------------
    if ((pcmReturn = snd_pcm_open(&pcmIn, captureDevice, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        std::cerr << "Capture open error: " << snd_strerror(pcmReturn) << std::endl;
        return -1;
    }

    snd_pcm_hw_params_malloc(&paramsIn);
    snd_pcm_hw_params_any(pcmIn, paramsIn);
    snd_pcm_hw_params_set_access(pcmIn, paramsIn, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcmIn, paramsIn, SND_PCM_FORMAT_S32_LE);
    snd_pcm_hw_params_set_channels(pcmIn, paramsIn, channels);
    snd_pcm_hw_params_set_rate_near(pcmIn, paramsIn, &sampleRate, nullptr);
    snd_pcm_hw_params_set_period_size_near(pcmIn, paramsIn, &periodSize, nullptr);
    snd_pcm_hw_params_set_buffer_size_near(pcmIn, paramsIn, &bufferSize);
    

    if ((pcmReturn = snd_pcm_hw_params(pcmIn, paramsIn)) < 0) {
        std::cerr << "Capture HW param error: " << snd_strerror(pcmReturn) << std::endl;
        return -1;
    }

    snd_pcm_hw_params_free(paramsIn);

    // --------------------------
    // SETUP PLAYBACK
    // --------------------------

    // Prints error if playback device is not detected
    if ((pcmReturn = snd_pcm_open(&pcmOut, playbackDevice, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        std::cerr << "Playback open error: " << snd_strerror(pcmReturn) << std::endl;
        return -1;
    }

    snd_pcm_hw_params_malloc(&paramsOut);
    snd_pcm_hw_params_any(pcmOut, paramsOut);
    snd_pcm_hw_params_set_access(pcmOut, paramsOut, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcmOut, paramsOut, SND_PCM_FORMAT_S32_LE);
    snd_pcm_hw_params_set_channels(pcmOut, paramsOut, channels);
    snd_pcm_hw_params_set_rate_near(pcmOut, paramsOut, &sampleRate, nullptr);
    snd_pcm_hw_params_set_period_size_near(pcmOut, paramsOut, &periodSize, nullptr);
    snd_pcm_hw_params_set_buffer_size_near(pcmOut, paramsOut, &bufferSize);

    if ((pcmReturn = snd_pcm_hw_params(pcmOut, paramsOut)) < 0) {
        std::cerr << "Playback HW param error: " << snd_strerror(pcmReturn) << std::endl;
        return -1;
    }

    snd_pcm_hw_params_free(paramsOut);


    // --------------------------
    // AUDIO BUFFER (32-bit)
    // --------------------------
    std::vector<int32_t> buffer(periodSize * channels);


    std::cout << "Audio input/output running. Press Ctrl+C to stop.\n";


    // --------------------------
    // MAIN LOOP
    // --------------------------
    while (keepRunning) {

        // Capture from input device
	pcmReturn = snd_pcm_readi(pcmIn, buffer.data(), periodSize);
	if (pcmReturn < 0) {
    	    snd_pcm_prepare(pcmIn);
    	    continue;
	}

        // ------------------------------------------------------------------
        //  PROCESSING BLOCK (your "true output" comes from here)
        //  ------------------------------------------------------
        //  buffer[] contains interleaved stereo samples in S32_LE format:
        //
        //      buffer[0] = L0
        //      buffer[1] = R0
        //      buffer[2] = L1
        //      buffer[3] = R1
        //      ...
        // ------------------------------------------------------------------

        // Example: mono mix → your DSP → output to both channels
        for (size_t i = 0; i < periodSize; i++) {
                // Convert stereo to  mono
                int32_t L = buffer[i * 2];
                int32_t R = buffer[i * 2 + 1];
                int32_t mono = (L / 2) + (R / 2);


                // Convert 32-bit int to float
                float f = (float)(mono / 2147483647.0f);
                
                // Include current sample for amplitude calculation
                ampCalc.add(f);

                // Low-pass filter
                f = LPF.filter(f);
                f = LPF.smoothing(f);

                // Log zero-crossing
                zeroCrossing.process(f);

                // Back to 32-bit integer
                int32_t processed = (int32_t)(f * 2147483647.0f);

                // Output to both channels
                buffer[i * 2]     = processed;
                buffer[i * 2 + 1] = processed;
        }
        
        // Calculate amplitude
        float amplitude = ampCalc.average();
        ampCalc.reset();

	// Detect transient
	bool transient = (amplitude - ampCalc.lastAmp) > ampCalc.threshold;
        ampCalc.lastAmp = amplitude;
        std::cout << amplitude << "amplitude\n";
        
	// Calculate 
        float duration = (float)periodSize / (float)SAMPLE_RATE;
        float frequency = (zeroCrossing.count / 2.0f) / duration;
        zeroCrossing.reset();

        note = midiConversion(frequency) + 12;
        
        // Trigger note on if transient detected
        if (transient == true){
	    noteOff = false;

	    // fraction-second note off
            velocity = 0;
            sendNote(oscTarget, false, lastNote, velocity);

	    velocity = 100;

            sendNote(oscTarget, true, note, velocity);
            lastNote = note;
        }

        // Trigger note off if note loudness is low enough and last note wasn't also a note off
        else if (amplitude < ampCalc.decayThreshold && noteOff == false){
            velocity = 0;
            sendNote(oscTarget, false, lastNote, velocity);
            noteOff = true;
        }

        // PLAYBACK
        pcmReturn = snd_pcm_writei(pcmOut, buffer.data(), periodSize);
        if (pcmReturn < 0) {
            snd_pcm_prepare(pcmOut);
            continue;
        }
    }

    std::cout << "\nStopping...\n";

    snd_pcm_drop(pcmIn);
    snd_pcm_close(pcmIn);

    snd_pcm_drop(pcmOut);
    snd_pcm_close(pcmOut);

    return 0;
}
