#!/bin/bash
#/mingw64/bin/qemu-system-i386.exe -fda ../bin/floppy.img
/mingw64/bin/qemu-system-i386.exe                       \
    -fda ../bin/floppy.img                              \
    -drive format=raw,file=../bin/floppy.img,if=floppy  \
    -S -s