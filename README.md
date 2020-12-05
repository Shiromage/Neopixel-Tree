# Neopixel-Tree
<i>Version 1.0</i>

A simple set of light effects for a strand of LEDs wrapped around a Christmas tree. See https://makeprojects.com/project/advanced-christmas-tree-lights

This project is developed on Visual Studio with PlatformIO. If you want to build this project with Arduino's IDE, you will need to shuffle some files around. Take all .cpp files from the src folder out and place it in the same directory as your Arduino sketch. You must also install the OctoWS2811 library and set the compile target board to Teensy 3.2. Using the Arduino IDE to compile for Teensy 3.2 requires a special tool from PJRC: https://www.pjrc.com/teensy/teensyduino.html

You will get "multiple definitions" errors if you have both main.cpp and the .ino file with main.cpp's content at the same time. Copy main.cpp's content into the sketch file and then delete main.cpp to fix this error.