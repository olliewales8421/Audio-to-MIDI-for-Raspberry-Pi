# Audio-to-MIDI-for-Raspberry-Pi
This repository uses zero-crossing pitch detection, OSC-over-IP, and Pure Data to achieve guitar-to-MIDI functionality via flagship-series Raspberry Pi without the direct MIDI capability found only in the smaller Pi models



Requirements:
    Hardware:
    
        32-bit float audio interface with USB-A cable
        Apple Mac (Should be compatible with other operating systems with adjustments to MIDI device and network setup)
        Ethernet cable
        Guitar or bass guitar with ¼ inch jack cable
    Software:
        Logic Pro (Should be compatible with other DAWs with adjustments to setup therein)
        Pure Data (https://puredata.info/downloads/pure-data) (14MB download)

Don’t plug in the Raspberry Pi until all previous steps are complete.
Exact instructions may differ per OS or version of Mac OS

Steps:
X:	Download the software package
X:	On the host computer, search “Audio MIDI Setup”
X:	Under the “Window” tab at the top of the screen, click “Show MIDI Studio”
X:	Double click on “IAC Driver”
X:	Within the “IAC Driver Properties” window next to “Add and Remove Ports”, use the + sign to create a new port
X:	Double click on the name of the newly created port to rename it (e.g. Pure Data MIDI)
X:	Check the “Device is online” checkbox and click “Apply”
X:	Making sure Pure Data is not already open, open the downloaded Pure Data patch “osc_to_midi.pd” (When you see a window with boxes and lines connecting them, that means the patch has been opened successfully)
X:	Under the “Media” tab, go to “MIDI Settings...” and change the output device to the MIDI device you created, then click “OK”
X:	Plug your audio interface into the Raspberry Pi
X:	Plug in the Raspberry Pi to a power source with the relevant power supply making sure that a solid red light and a flashing orange light appear on the Pi around the corner from the power cable
X:	Connect your computer to the Raspberry Pi with your ethernet cable and approve any dialogue boxes that may appear
X:	In your computer’s network settings, find the ethernet connection (e.g. USB 10/100/1000 LAN) and click “Details”
X:	Under TCP/IP, change “Configure IPv4” (Not IPv6) to “Manually” and change the IP address to “192.168.2.1”, leaving all other sections as their defaults
X:	Open Logic and create a “Software Instrument” track
X:	Make sure that an appropriate instrument is loaded on the track
X:	Plug your guitar or bass into the audio interface
X:	If your guitar or bass has a tone control, make sure it’s rolled all the way off to help note accuracy
X:	Make sure that the input gain on the audio interface is not all the way down and make adjustments until you get a suitable response in the DAW (e.g. turn up the input gain if it’s not sensitive enough, and turn it down if it’s too sensitive)
