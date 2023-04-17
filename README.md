# rpi4-esp32c3-labs

#### Install packages for ESP32C2

In a separate directory, install a package. Follow instruction: <br/>
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html
```
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd ~/esp/esp-idf
./install.sh esp32c3
```
Once installed, allow "idf.py" command
```
cd ~/esp/esp-idf
. export.sh
```


#### flash and monitor
```
#Build
cd ~/esp/<file-name>
idf.py set-target esp32c3
cd build ninja
cd ~/esp/<file-name>
idf.py flash monitor
```



