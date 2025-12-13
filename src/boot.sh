#!/bin/bash
#/mingw64/bin/qemu-system-i386.exe -fda ../bin/floppy.img
#/mingw64/bin/qemu-system-i386.exe                       \
#    -drive file=../bin/floppy.img,if=floppy,unit=0,format=raw
#    -S -gdb tcp:127.0.0.1:12345

#/mingw64/bin/qemu-system-i386.exe -hda ../bin/hdd.img -m 256 -boot c
#/mingw64/bin/qemu-system-i386.exe -drive file=../bin/hdd.img,format=raw -m 256
/mingw64/bin/qemu-system-i386.exe -m 512M -boot d -cdrom ../bin/bootcd.iso
