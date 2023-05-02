# Humidity and Temperature

This program prints the direction pointed by the ESP32C3 board using inclination. It will also print out a diagonal directions by mixing up the horizontal with vertical if the board is slanted in a cross sectional direction. Possible options are UP, DOWN, LEFT, and RIGHT.

## contents

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── lab2_asm.txt
├── sdkconfig
├── main
│   ├── CMakeLists.txt
│   └── lab3_1.c
└── README.md                  This is the file you are currently reading
```

## Notes

Most of the code was written by chatGPT and transcribed by hand from professor Renau's lecture. I modified the program to read direction using gyro_data_x and gyro_data_y determined by the ICM-42670-P component.