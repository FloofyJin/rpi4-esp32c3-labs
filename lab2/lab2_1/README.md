# print value with GDB

This program is a sample code that was used to read an .elf file (eg. lab2_image.elf) and debug the function compute that is stored in it.

## contents

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── lab2_asm.txt
├── sdkconfig
├── main
│   ├── CMakeLists.txt
│   └── lab2_1.c
└── README.md                  This is the file you are currently reading
```

## Notes

The steps to replicate the behavior:

* idf.py set-target esp32c3<br/>
* idf.py build<br/>
* replace .elf file in the build folder<br/>
* idf.py flash<br/>


Or just simply run esptool.py which will flash the board

* (esptool.py](https://docs.espressif.com/projects/esptool/en/latest/esp32s3/esptool/basic-commands.html#convert-elf-to-binary-elf2image)

Then

* riscv-esp-elf-gdb -x gdbinit build/lab2_1.elf<br/>

