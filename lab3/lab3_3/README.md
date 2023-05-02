# Humidity and Temperature

This program will allow user to connect to it via bluetooth on a computer or phone that is running mac, android, or windows11. (Windows10 does not seem to be supported) Once connected, it will allow the user to control the mouse by tilting the ESP32C3 board in all 360 degree direction.

## contents

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── esp_hidd_prf_api.c
├── esp_hidd_prf_api.h
├── hid_dev.c
├── hid_dev.h
├── hid_device_le_prf.c
├── hidd_le_prf_int.h
├── sdkconfig
├── main
│   ├── CMakeLists.txt
│   └── ble_hidd_demo_main.c
└── README.md                  This is the file you are currently reading
```

## Notes

This code was exported from [https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/bluedroid/ble/ble_hid_device_demo/main/ble_hidd_demo_main.c](esp ble_hidd_demo). The code utilizes lab3_1 and lab3_2.