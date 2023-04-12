# Blink LED

This example demonstrates how to blink a LED using GPIO or using either GPIO2 or GPIO7 on the ESP32C3 board

### Configure the Project to use the correct GPIO

Open the project configuration menu (`idf.py menuconfig`).

In the `Example Configuration` menu:

* Select the LED type in the `Blink LED type` option.
  * Use `GPIO` for regular LED blink.
* Set the GPIO number used for the signal in the `Blink GPIO number` option.
* Set the blinking period in the `Blink period in ms` option.

### Build and Flash

Run `idf.py flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)


### Notes

I had issues with using GPIO2 (white LED in the middle), therefore this program will only blink GPIO2 instead. GPIO2 requires additiional configuration in the menuconfig.

