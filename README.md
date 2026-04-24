# Audio-to-MIDI-for-Raspberry-Pi
This repository uses zero-crossing pitch detection, OSC-over-IP, and Pure Data to achieve guitar-to-MIDI functionality via flagship-series Raspberry Pi without the direct MIDI capability found only in the smaller Pi models.

## Setup for if you have a configured Raspberry Pi
[ if not, see further down ]
### Requirements:
Hardware:
- 32-bit float audio interface with USB-A cable
- Apple Mac (Should be compatible with other operating systems with adjustments to MIDI device and network setup)
- Ethernet cable
- Guitar or bass guitar with ¼ inch jack cable

Software:
- Logic Pro (Should be compatible with other DAWs with adjustments to setup therein)
- Pure Data (https://puredata.info/downloads/pure-data) (14MB download)

\
_Don’t plug in the Raspberry Pi until all previous steps are complete_\
_Exact instructions may differ per OS or version of Mac OS_

### Steps:
1.  Right-click on any chosen folder on the host computer and click "New Terminal at Folder"
2.	Clone the repository by pasting this line into the terminal
    ```
    git clone https://github.com/olliewales8421/Audio-to-MIDI-for-Raspberry-Pi
    ```

3.	On the host computer, search “Audio MIDI Setup”
4.	Under the “Window” tab at the top of the screen, click “Show MIDI Studio”
5.	Double click on “IAC Driver”
6.	Within the “IAC Driver Properties” window next to “Add and Remove Ports”, use the + sign to create a new port
7.	Double click on the name of the newly created port to rename it (e.g. Pure Data MIDI)
8.	Check the “Device is online” checkbox and click “Apply”
9.	Making sure Pure Data is not already open, go to the cloned repository and open the Pure Data patch “osc_to_midi.pd” (When you see a window with boxes and lines connecting them, that means the patch has been opened successfully)
10.	Under the “Media” tab, go to “MIDI Settings...” and change the output device to the MIDI device you created, then click “OK”
11.	Plug your audio interface into the Raspberry Pi
12.	_Plug in the Raspberry Pi to a power source with the relevant power supply making sure that a solid red light and a flashing orange light appear on the Pi around the corner from the power cable_
13.	Connect your computer to the Raspberry Pi with your ethernet cable and approve any dialogue boxes that may appear
14.	In your computer’s network settings, find the ethernet connection (e.g. USB 10/100/1000 LAN) and click “Details”
15.	Under TCP/IP, change “Configure IPv4” (Not IPv6) to “Manually” and change the IP address to “192.168.2.1”, leaving all other sections as their defaults
16.	Open Logic and create a “Software Instrument” track
17.	Make sure that an appropriate instrument is loaded on the track
18.	Plug your guitar or bass into the audio interface
19.	If your guitar or bass has a tone control, make sure it’s rolled all the way off to help note accuracy
20.	Make sure that the input gain on the audio interface is not all the way down and make adjustments until you get a suitable response in the DAW (e.g. turn up the input gain if it’s not sensitive enough, and turn it down if it’s too sensitive)

## Setup for if you want to configure your Raspberry Pi
### Requirements:
Hardware:
- Raspberry Pi with a USB port, ethernet port, and an operating system installed (e.g. Raspbian)
- Ethernet cable

Software:
- A C++ compiler (e.g. Clang)
- ALSA audio system
- liblo

### Steps:
1.  Power on the Raspberry Pi and connect it to the host computer via the ethernet cable
2.  In the host laptop terminal, SSH into the Pi
    ```
    ssh raspberrypi.local
    ```
3.  Clone the repository
    ```
    git clone https://github.com/olliewales8421/Audio-to-MIDI-for-Raspberry-Pi
    ```
4.  Navigate to the repository directory
    ```
    cd Audio-to-MIDI-for-Raspberry-Pi
    ```
5.  Compile the main program
    ```
    clang++ main.cpp -o main -lasound -lm -llo
    ```
6.  Using 
    ```
    sudo nmtui
    ```
    navigate to the ethernet connection and manually change the IPv4 address to 192.168.2.2/24, and the gateway and DNS server to 192.168.2.1
7.  Use
    ```
    crontab -e
    ```
    and add the line
    ```
    @reboot sudo Audio-to-MIDI-for-Raspberry-Pi/main 192.168.2.1 9000 &
8.  Close the connection and power off the Pi
9.  Follow the steps of the section further up
    
    
