# -----------------------------------------------------------------------------
# \file  code32.asm
# \note  (c) 2025 by Jens Kallup - paule32
#        all rights reserved.
# -----------------------------------------------------------------------------
CROSS    ?= 

AS       := nasm
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
LDSHFLGS := -nostdlib -T ../src/shell.ld  -Map ../bin/shell.map

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
SRCC := $(SRC)/ckernel.c        \
        $(SRC)/paging.c         \
        $(SRC)/idt.c            \
        $(SRC)/irqc.c           \
        $(SRC)/isrc.c           \
        $(SRC)/gdt.c            \
        $(SRC)/syscall.c        \
        $(SRC)/timer.c          \
        $(SRC)/usermodec.c      \
        $(SRC)/task.c           \
        $(SRC)/iso9660.c        \
        $(SRC)/video.c          \
        $(SRC)/util.c           \
        $(SRC)/atapi.c          \
        $(SRC)/ahci.c           \
        $(SRC)/shell_loader.c   \
        $(SRC)/pe.c             \
        $(SRC)/kheap.c
# -----------------------------------------------------------------------------
# source files for the C-kernel ...
# -----------------------------------------------------------------------------
SHSRCC := $(SRC)/shell.c

# -----------------------------------------------------------------------------
# *.o bject files for linkage stage of the C-kernel ...
# -----------------------------------------------------------------------------
OBJS := $(OBJ)/ckernel.o        \
        $(OBJ)/paging.o         \
        $(OBJ)/flush.o          \
        $(OBJ)/idt.o            \
        $(OBJ)/isr.o            \
        $(OBJ)/irqc.o           \
        $(OBJ)/irq.o            \
        $(OBJ)/isrc.o           \
        $(OBJ)/gdt.o            \
        $(OBJ)/syscall.o        \
        $(OBJ)/timer.o          \
        $(OBJ)/usermode.o       \
        $(OBJ)/usermodec.o      \
        $(OBJ)/task.o           \
        $(OBJ)/iso9660.o        \
        $(OBJ)/video.o          \
        $(OBJ)/util.o           \
        $(OBJ)/atapi.o          \
        $(OBJ)/ahci.o           \
        $(OBJ)/shell_loader.o   \
        $(OBJ)/pe.o             \
        $(OBJ)/kheap.o
# -----------------------------------------------------------------------------
# *.o bject files for linkage stage of the shell ...
# -----------------------------------------------------------------------------
SHOBJS := \
        $(OBJ)/shell.o  \
        $(OBJ)/kheap.o  \
        $(OBJ)/vga.o    \
        $(OBJ)/video.o  \
        $(OBJ)/util.o

LBA        := $(SRC)/lba.inc
ISO        := $(BIN)/bootcd.iso
ISO_FILES  := /boot2.bin /kernel.bin

all: config\
     $(OBJ)/data.o $(BIN)/bootcd.iso

# -----------------------------------------------------------------------------
# Ziel-Datei mit den extrahierten Infos
# -----------------------------------------------------------------------------
define GEN-LBA
	@echo "Create $(LBA) from $(ISO)..."
	if [ -f   $(LBA) ]; then  \
        rm    $(LBA); \
        touch $(LBA); \
    fi
	@echo "create default $(LBA)..."
	@echo ";---------------------------------------------------------" >> $(LBA)
	@echo "; Diese beiden muessen wir *patchen*" >> $(LBA)
	@echo ";---------------------------------------------------------" >> $(LBA)
	@echo "TEST_LBA       equ 0" >> $(LBA)
	@echo "" >> $(LBA)
	@echo "BOOT2_LBA      equ 0" >> $(LBA)
	@echo "BOOT2_SECTORS  equ 0" >> $(LBA)
	@echo ";---------------------------------------------------------" >> $(LBA)
	@echo "; Konstanten – mit Werten aus xorriso ausfüllen"            >> $(LBA)
	@echo ";---------------------------------------------------------" >> $(LBA)
	@echo "KERNEL_LBA      equ 0" >> $(LBA)
	@echo "KERNEL_SECTORS  equ 0" >> $(LBA)
	@echo "" >> $(LBA)
	@echo "VBE_INFO_ADDR   equ 0x0A00" >> $(LBA)
endef
define MOD-LBA
	@echo "Modify $(LBA) from $(ISO)..."
	if [ -f   $(LBA) ]; then  \
        rm    $(LBA); \
        touch $(LBA); \
    fi
	@echo "; -----------------------------------------------------" >> $(LBA)
	@echo "; DONT CHANGE THIS FILE - all modifieds will be lost.  " >> $(LBA)
	@echo "; -----------------------------------------------------" >> $(LBA)
	@echo "TEST_LBA       equ 0" >> $(LBA)
	@>> $(LBA)
	@for f in $(ISO_FILES); do \
        echo "search $$f";     \
        xorriso -indev $(ISO) -find / -exec report_lba -- \
        | awk -v iso="$$f" '/^File data lba:/ { \
            path = $$NF;       \
            gsub(/^'\''|'\''$$/, "", path);  \
            if (path == iso) {               \
                name = path;                 \
                sub(/^\/+/      , "", name); \
                sub(/\.[^./]*$$/, "", name); \
                name = toupper(name); \
                print name "_LBA     equ ", $$6 "   ; start LBA ", name; \
                print name "_SECTORS equ ", $$8 "   ; 2048 * "   , $$8 ; \
                print "; -----------------------------------------------------"; \
            }      \
	    }'  >> $(LBA); \
    done
	@echo "VBE_INFO_ADDR   equ 0x0A00" >> $(LBA)
	$(AS) -f bin $(SRC)/boot1.asm -o $(BIN)/boot1.bin
	$(AS) -f bin $(SRC)/boot2.asm -o $(BIN)/boot2.bin
	cp $(BIN)/boot1.bin content/boot1.bin
	cp $(BIN)/boot2.bin content/boot2.bin
	@cd content && \
	xorriso -as mkisofs -o ../../bin/bootcd.iso \
        -b boot1.bin        \
        -no-emul-boot       \
        -boot-load-size 4   \
        . && \
    cd ..
	@echo "DONE"
endef
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

$(BIN)/INITRD.EXE: $(OBJ)/make_initrd.o
	$(GCC) -m32 -mconsole -O2 -Wall -Wextra -o $@ $<
	$(STRIP) $@

$(BIN)/kernel.bin: $(OBJ)/kernel.o
	$(OBJCOPY) -O binary $(OBJ)/kernel.o $(BIN)/kernel.bin

$(OBJ)/shell.bin: $(SHOBJS) $(SRC)/shell.ld
	$(LD) $(LDSHFLGS) -o    $(OBJ)/shell.bin $(SHOBJS)

$(BIN)/shell.exe:  $(OBJ)/shell.bin
	$(OBJCOPY) -O binary $(OBJ)/shell.bin $(BIN)/shell.exe

$(BIN)/bootcd.iso: $(BIN)/boot1.bin $(BIN)/boot2.bin $(BIN)/kernel.bin $(BIN)/shell.exe
	./create_iso.sh
	$(MOD-LBA)

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
	$(GEN-LBA)

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
	$(RM) -f  $(BIN)/hdd.img $(BIN)/shell.exe
	$(RM) -rf $(OBJ)
