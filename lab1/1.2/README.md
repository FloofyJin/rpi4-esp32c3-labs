# Hello World + Printing My Name

This project utilizes FreeRTOS task to print "Hello World" every 10 seconds. By modifying a line under where it prints "Heaps[...]", I added a printf() statement for my name.

## contents

The project **hello_world** contains one source file in C language [hello_world_main.c](main/hello_world_main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt` files that provide set of directives and instructions describing the project's source files and targets (executable, library, or both).

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── pytest_hello_world.py      Python script used for automated testing
├── main
│   ├── CMakeLists.txt
│   └── hello_world_main.c
└── README.md                  This is the file you are currently reading
```

## Notes

Nothing important to note. but I have made a list of steps to run the following program using idf.py

Connect power to Raspberry pi4 <br/>
Connect esp32c3  to Raspberry pi4 <br/>
Inside Raspberry terminal, <br/>
```
cd ~/esp/esp-idf
./install.sh esp32c3
cd ~/esp/esp-idf
. export.sh
cd ~/esp/hello_world
idf.py set-target esp32c3
cd build
ninja
cd ~/esp/hello_world
idf.py flash monitor
```
