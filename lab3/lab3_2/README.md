# Humidity and Temperature

This program will allow user to connect to it via bluetooth on a computer or phone that is running mac, android, or windows11. (Windows10 does not seem to be supported) Once connected, it will move the mouse right for 2 seconds, left 2 seconds, and wait 5 seconds before repeating the cycle again.

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

This code was exported from [https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/bluedroid/ble/ble_hid_device_demo/main/ble_hidd_demo_main.c](esp ble_hidd_demo). I modified to the code to move the mouse using the command ``esp_hidd_send_mouse_value(hid_conn_id, mouse, x, y)``