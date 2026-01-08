#!/bin/bash
echo "-------------------------"
COR_DIR=$__COR_DIR
BIN_DIR=$__BIN_DIR
OBJ_DIR=$__OBJ_DIR
# ----------------
AS=$__BIN_AS

objcopy -O binary ${OBJ_DIR}/kernel.o ${BIN_DIR}/kernel.bin
ls -l ${BIN_DIR}

nasm -DLBA_FILE="${COR_DIR}/lba.inc"    \
    -f bin ${COR_DIR}/boot/boot1.asm    \
    -o     ${BIN_DIR}/content/boot1.bin \
    -l     ${BIN_DIR}/boot1.lst \
    -s     ${BIN_DIR}/boot1.sym

nasm -DLBA_FILE="${COR_DIR}/lba.inc"    \
    -f bin ${COR_DIR}/boot/boot2.asm    \
    -o     ${BIN_DIR}/content/boot2.bin \
    -l     ${BIN_DIR}/boot2.lst \
    -s     ${BIN_DIR}/boot2.sym

dd if=/dev/zero of=${BIN_DIR}/hdd.img bs=1M count=16
dd if=${BIN_DIR}/content/boot1.bin of=${BIN_DIR}/hdd.img bs=512 count=1 conv=notrunc
dd if=${BIN_DIR}/hdd.img bs=1 skip=510 count=2 2>/dev/null | od -An -t x1 | \
    awk '{ if ($1 == "55" && $2 == "aa") {
        print"OK: Signature: "$1" "$2" found.";
    }   else {
        print "ERROR: ("$1" "$2") not found.";
        exit 1;
    }}'

cp ${BIN_DIR}/kernel.bin ${BIN_DIR}/content/kernel.bin
cp ${BIN_DIR}/shell.exe  ${BIN_DIR}/content/shell.exe

cd ${BIN_DIR}/content
xorriso -as mkisofs -o ${BIN_DIR}/bootcd.iso \
    -b boot1.bin        \
    -no-emul-boot       \
    -boot-load-size 4   \
    .
cd ..
xorriso -indev ${BIN_DIR}/bootcd.iso -find / -exec report_lba --
