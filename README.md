# Spacelady

[Spacelady](http://jdeboi.com/portfolio/spacelady-mural/) is an interactive, Arduino-controlled mural. Using a capacitive touch sensor, the Arduino triggers lights and sounds when copper tape pads are touched. To understand the functionality of this mural and to see it in action, watch the videos in the links below

####Libraries
Before uploading the Spacelady.ino to the Arduino board, two libraries need to be installed in the libraries folder (follow the instructions in the links below):
- [Adafruit Neopixel library](https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library)
- [Adafruit Capacitive sensor library](https://github.com/adafruit/Adafruit_MPR121_Library)

####Sound
If you want Soundlady to have sound buttons, you have to make sure to install [Processing](https://processing.org/).
Inside the SoundPlayer folder, open SoundPlayer.pde with Processing and run the sketch after you've got the Arduino code working & while the Arduino is plugged into your computer. In Soundplayer.pde, select the port the Arduino is running on (usually something like usbmodem1411 on a Mac). You should now hear sounds when you touch the "sound" buttons (check the Arduino code to find out which inputs trigger sounds). The Arduino is sending serial data to Processing, which is the software that's generating the sound.

To change the sound files, simply replace the files in the SoundPlayer/audio folder with your own .wav files (named 0-11.wav).

####More Info
For step-by-step instructions, check out my [Instructable](http://www.instructables.com/id/Interactive-Arduino-Mural/).
For links and videos, check out [my website](http://jdeboi.com/portfolio/spacelady-mural/).