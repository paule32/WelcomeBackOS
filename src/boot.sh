#!/bin/bash
#/mingw64/bin/qemu-system-i386.exe -fda ../bin/floppy.img
#/mingw64/bin/qemu-system-i386.exe                       \
#    -drive file=../bin/floppy.img,if=floppy,unit=0,format=raw
#    -S -gdb tcp:127.0.0.1:12345

#/mingw64/bin/qemu-system-i386.exe -hda ../bin/hdd.img -m 256 -boot c
#/mingw64/bin/qemu-system-i386.exe -drive file=../bin/hdd.img,format=raw -m 256

#/mingw64/bin/qemu-system-i386.exe -m 256 -boot d -cdrom ../bin/bootcd.iso

#/mingw64/bin/qemu-system-i386.exe \
#    -drive id=disk,file=dummy.img,format=raw,if=ide \
#    -drive id=cdrom,file=boot.iso,if=ide,media=cdrom \
#    -boot d -m 512

/mingw64/bin/qemu-system-x86_64.exe \
    -drive file=../bin/bootcd.iso,if=none,media=cdrom,id=cdrom0 \
    -device ich9-ahci,id=ahci0 \
    -device ide-cd,drive=cdrom0,bus=ahci0.0 \
    -boot d -m 256M -machine q35,i8042=on

#-net nic,model=e1000 -net user