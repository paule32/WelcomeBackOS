# -----------------------------------------------------------------------------
# \file  code32.asm
# \note  (c) 2025 by Jens Kallup - paule32
#        all rights reserved.
# -----------------------------------------------------------------------------
CROSS    ?= 

AS       := ../bin/nasm
ECHO     := @echo
EXPORT   := export
GZIP     := gzip
RM       := rm
MKDIR    := mkdir
STRIP    := strip
COPY     := cp

GCC      := $(CROSS)gcc
CPP      := $(CROSS)g++

LD       := $(CROSS)ld
OBJCOPY  := $(CROSS)objcopy

# -----------------------------------------------------------------------------
# compiler flags ...
# -----------------------------------------------------------------------------
CFLAGS   := -m32 -O1 -ffreestanding -fno-stack-protector -fno-builtin   \
            -nostdlib -nostartfiles -fno-pic -Wall -Wextra              \
            -mno-ms-bitfields -Wno-unused-variable                      \
            -Wno-unused-value -Wno-unused-parameter -Wno-sign-compare   \
            -I../src/include

CPPFLAGS := -std=c++20

LDFLAGS  := -nostdlib -T ../src/kernel.ld -Map ../bin/kernel.map
ASMFLAGS := -f win32 -O2 -DDOS_MODE=32

# -----------------------------------------------------------------------------
# input/output directories to hold the source tree clean ...
# -----------------------------------------------------------------------------
SRC  := ../src
OBJ  := ../obj
BIN  := ../bin

# -----------------------------------------------------------------------------
# source files for the C-kernel ...
# -----------------------------------------------------------------------------
SRCC := $(SRC)/ckernel.c            \
        $(SRC)/paging.c             \
        $(SRC)/idt.c                \
        $(SRC)/isrc.c               \
        $(SRC)/syscall.c            \
        $(SRC)/kheap.c

# -----------------------------------------------------------------------------
# *.o bject files for linkage stage of the C-kernel ...
# -----------------------------------------------------------------------------
OBJS := $(OBJ)/ckernel.o            \
        $(OBJ)/paging.o             \
        $(OBJ)/flush.o              \
        $(OBJ)/idt.o                \
        $(OBJ)/isr.o                \
        $(OBJ)/isrc.o               \
        $(OBJ)/syscall.o            \
        $(OBJ)/kheap.o

all: config\
     $(OBJ)/data.o $(BIN)/bootcd.iso

# -----------------------------------------------------------------------------
# loader to load the system from DOS console ...
# -----------------------------------------------------------------------------
$(BIN)/boot1.bin: $(SRC)/boot1.asm
	$(AS) -f bin  $(SRC)/boot1.asm -o $(BIN)/boot1.bin

$(BIN)/boot2.bin: $(SRC)/boot2.asm
	$(AS) -f bin  $(SRC)/boot2.asm -o $(BIN)/boot2.bin

# -----------------------------------------------------------------------------
# compile all *.c files to *.o bject files ...
# -----------------------------------------------------------------------------
$(OBJ)/%.o: $(SRC)/%.c
	$(GCC) $(CFLAGS) -c $< -o $@
    
$(OBJ)/%.o: $(SRC)/%.cc
	$(CPP) $(CFLAGS) -c $< -o $@

# -----------------------------------------------------------------------------
# compile all *.asm files to *.o bject files ...
# -----------------------------------------------------------------------------
$(OBJ)/%.o: $(SRC)/%.asm
	$(AS) $(ASMFLAGS) $< -o $@

# -----------------------------------------------------------------------------
# link C-kernel to finaly output binary image ...
# -----------------------------------------------------------------------------
$(OBJ)/kernel.o: $(OBJS) $(SRC)/kernel.ld
	$(LD) $(LDFLAGS) -o  $(OBJ)/kernel.o $(OBJS)

# -----------------------------------------------------------------------------
$(OBJ)/data.o: $(SRC)/initrd.dat $(SRC)/data.asm
	$(AS) $(ASMFLAGS) $(SRC)/data.asm -o $(OBJ)/data.o
    
# -----------------------------------------------------------------------------
# create the initial ram disk ...
# -----------------------------------------------------------------------------
$(SRC)/initrd.dat: $(BIN)/INITRD.EXE
	$(BIN)/INITRD.EXE \
    test1.txt   file1 \
    test2.txt   file2 \
    test3.txt   file3

#    $(SRC)/content/program.elf shell

$(BIN)/INITRD.EXE: $(OBJ)/make_initrd.o
	$(GCC) -m32 -mconsole -O2 -Wall -Wextra -o $@ $<
	$(STRIP) $@

$(BIN)/kernel.bin: $(OBJ)/kernel.o
	$(OBJCOPY) -O binary $(OBJ)/kernel.o $(BIN)/kernel.bin

$(BIN)/bootcd.iso: $(BIN)/boot1.bin $(BIN)/boot2.bin $(BIN)/kernel.bin
	./create_iso.sh

$(BIN)/floppy.img: $(BIN)/BOOT1.SYS $(BIN)/BOOT2.SYS $(BIN)/KERNEL.SYS
	dd if=/dev/zero of=$(BIN)/floppy.img bs=1024 count=1440
	mformat -t 80 -h 2 -s 18 -f 1440 \
        -B ../bin/BOOT1.SYS \
        -i ../bin/floppy.img ::
	@dd if=$(BIN)/floppy.img bs=1 skip=510 count=2 2>/dev/null | \
        od -An -t x1 | \
        awk '{ if ($$1 == "55" && $$2 == "aa") { \
            print "OK: Bootsignatur 55 aa found."; \
            exit 0;  \
	    }   else {   \
            print "Error: Bootsignatur incorrect. (" $$1 " " $$2 ")"; \
            exit 1;  \
	    } }'
	cp $(BIN)/BOOT2.SYS $(SRC)/content/BOOT2.SYS
	mcopy -i $(BIN)/floppy.img $(SRC)/content/* ::/

# -----------------------------------------------------------------------------
# prepare font ...
# -----------------------------------------------------------------------------
$(SRC)/unifont.hex: $(SRC)/unifont8x16.hex $(SRC)/make_unifont8x16.py
	python3 $(SRC)/make_unifont8x16.py unifont8x16.hex font8x16.h

$(SRC)/unifont8x16.hex:
	wget --no-check-certificate \
    https://mirrors.kernel.org/gnu/unifont/unifont-15.1.05/unifont_all-15.1.05.hex.gz
	gunzip unifont_all-15.1.05.hex.gz
	mv $(SRC)/unifont_all-15.1.05.hex $(SRC)/unifont8x16.hex

# -----------------------------------------------------------------------------
# create directories for output object code ...
# -----------------------------------------------------------------------------
config: $(SRC)/unifont.hex
	$(MKDIR) -p $(BIN)
	$(MKDIR) -p $(OBJ)

# -----------------------------------------------------------------------------
# before you compile, you can clean the source tree from old crap ...
# -----------------------------------------------------------------------------
clean:
	$(RM) -f  $(OBJ)/*.o $(OBJ)/*.coff
	$(RM) -f  $(BIN)/*.BIN $(BIN)/*.COM $(BIN)/*.sys $(BIN)/kernel.map
	$(RM) -f  $(SRC)/content/*.sys $(SRC)/content/*.bin
	$(RM) -f  $(SRC)/unifont_all-15.1.05.hex
	$(RM) -f  $(BIN)/bootcd.iso
	$(RM) -f  $(SRC)/unifont.hex
	$(RM) -f  $(SRC)/initrd.dat
	$(RM) -f  $(BIN)/hdd.img
	$(RM) -rf $(OBJ)
