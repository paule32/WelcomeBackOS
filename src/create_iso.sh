#!/bin/bash

objcopy -O binary ../obj/kernel.o ../bin/kernel.bin
ls -l ../bin
../bin/nasm -f bin ../src/boot1.asm -o ../bin/boot1.bin
../bin/nasm -f bin ../src/boot2.asm -o ../bin/boot2.bin

dd if=/dev/zero of=../bin/hdd.img bs=1M count=16
dd if=../bin/boot1.bin of=../bin/hdd.img bs=512 count=1 conv=notrunc
dd if=../bin/hdd.img bs=1 skip=510 count=2 2>/dev/null | od -An -t x1 | \
    awk '{ if ($1 == "55" && $2 == "aa") {
        print"OK: Signature: "$1" "$2" found.";
    }   else {
        print "ERROR: ("$1" "$2") not found.";
        exit 1;
    }}'

cp ../bin/boot1.bin  content/boot1.bin
cp ../bin/boot2.bin  content/boot2.bin
cp ../bin/kernel.bin content/kernel.bin

cd content
xorriso -as mkisofs -o ../../bin/bootcd.iso \
    -b boot1.bin        \
    -no-emul-boot       \
    -boot-load-size 4   \
    .
cd ..
xorriso -indev ../bin/bootcd.iso -find / -exec report_lba --
/mingw64/bin/qemu-system-i386.exe -m 256 -boot d -cdrom ../bin/bootcd.iso
