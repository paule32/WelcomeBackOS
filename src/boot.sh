#!/bin/bash
#/mingw64/bin/qemu-system-i386.exe -fda ../bin/floppy.img
/mingw64/bin/qemu-system-i386.exe                       \
    -fda ../bin/floppy.img                              \
    -drive format=raw,file=../bin/floppy.img,if=floppy
#    -S -gdb tcp:127.0.0.1:12345
