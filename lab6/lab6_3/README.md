## Morse LED in PI4

This program is intended with work with ADA, ground, and 3v3/5v pins on the ESP32C3 board. It  will take in light input from an external source. This device will read the blinking light through the photoresistor led that is hooked up to the ESP32C3 board.

### Running
In one terminal, run ESP32C3 instance
```
idf.py set-target esp32c3
idf.py build flash monitor
```
In second terminal, run the C program that will blinkg the LED. The LED was hooked to the raspberry pi 4 in my demo. The C program will need to be modified depending on the dot/dash/space time that is specified in the lab6_3.c of this program.