| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

# Wi-Fi Station Example

This will connect to hotspot and send temp/humidity to a server on the hotspot. It received location from the server through a GET request. By that, it will request city temperature via wttr.in website. If there is no hotspot. Comment out ``strcpy(rpi_server, gw_ip_str);`` and uncomment ``strcpy(rpi_server, "192.168.0.48");``. Replace the ip address with the ip address of the server.

### Configure the project

Open the project configuration menu (`idf.py menuconfig`).

In the `ssid` menu:

* Set the Wi-Fi configuration.
    * Set `WiFi SSID`.
    * Set `WiFi Password`.

### Build and Flash

Create build files:

Run `idf.py set-target esp32c3`

Build the project and flash it to the board, then run the monitor tool to view the serial output:

Run `idf.py build flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)


## Example Output
