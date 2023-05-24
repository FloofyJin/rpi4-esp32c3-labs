## Morse LED in PI4

This program is intended with work with GPIO16 and ground pin on the raspberry pi. It will take in an argument: number of times to repeat and the string to convert to morse code.

```
make
./send 4 "hello ESP32"
```

### installation
This program was intended to be run on an ubuntu 20.04 server running on raspberry pi 4.

You need libgpiod-dev and gcc package installed.

```
sudo apt update
sudo apt install libgpiod-dev
```
You might need to reboot after installation


### Testing
There is a python script test.py that will blink on/off every 5 seconds. This is purely for testing only.