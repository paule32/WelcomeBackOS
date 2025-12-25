# -----------------------------------------------------------------------------
# \file  Makefile
# \note  (c) 2025 by Jens Kallup - paule32
#        all rights reserved.
# -----------------------------------------------------------------------------
# we only support MinGW 32-bit and Linux Systems ...
# -----------------------------------------------------------------------------
MAKE_VERSION_LINE := $(shell $(MAKE) --version 2>/dev/null | head -n 1)
MAKE_VENDOR       := $(word 1,$(MAKE_VERSION_LINE))
MAKE_VERSION      := $(word 3,$(MAKE_VERSION_LINE))
# -----------------------------------------------------------------------------
AUTO_YES          ?= 0  # interactive/remote (1) or local (0) ?
# -----------------------------------------------------------------------------
ComSpec           ?= /C/Windows/System32/cmd.exe
# -----------------------------------------------------------------------------
DATE_YMD := $(shell date +%F)    # yyyy-mm-dd
TIME_HMS := $(shell date +%T)    # HH:MM:SS
# -----------------------------------------------------------------------------
DATE_YMD := $(strip $(DATE_YMD))
TIME_HMS := $(strip $(TIME_HMS))
# -----------------------------------------------------------------------------
ifneq ($(MAKE_VENDOR),GNU)
    $(error Error: only support GNU make for MinGW-32 or Linux.)
endif
ifeq (,$(PSModulePath))
    SHELL_KIND := powershell
    $(error Error: PowerShell Environment not supported.)
endif
ifneq (,$(SHELL))
    SHELL_KIND := bash
endif
ifneq ($(SHELL_KIND),bash)
    $(error Error: SHELL_KIND is not set to Unix-like Console.)
else
    ifeq ($(OS),Windows_NT)
        HOST_OS := windows
    else
        HOST_OS := unix
    endif
endif
# -----------------------------------------------------------------------------
# double sanity check ...
# -----------------------------------------------------------------------------
ifeq ($(HOST_OS),windows)
    UNAME_S := $(strip $(shell uname -s 2>/dev/null))
    ifneq ($(UNAME_S),)
        # MSYS2/Git-Bash/MINGW-Umgebung
        ENV_STYLE := unix
    else
        # cmd/powershell ohne uname/coreutils
        ENV_STYLE := win32
        $(error Error: ENV_STYLE not unix-like.)
    endif
endif
# -----------------------------------------------------------------------------
# detect build directory ...
# -----------------------------------------------------------------------------
CURDIR          := $(shell pwd)
CURDIR_ABS      := $(abspath $(CURDIR))
IS_IN_BUILD     := $(filter %/build,$(CURDIR_ABS))

PARENT_OF_BUILD := $(patsubst %/build,%,$(CURDIR_ABS))
BUILDS_OF_BUILD := $(PARENT_OF_BUILD)/build

SOURCE_OF_BUILD := $(PARENT_OF_BUILD)/src
OBJECT_OF_BUILD := $(BUILDS_OF_BUILD)/obj

CORE32_OF_BUILD := $(SOURCE_OF_BUILD)/kernel

# -----------------------------------------------------------------------------
# default: current working directory ...
# -----------------------------------------------------------------------------
BASEDIR := $(CURDIR_ABS)

# -----------------------------------------------------------------------------
# switch only if we are in /build AND parent/src exists ...
# -----------------------------------------------------------------------------
ifneq ($(IS_IN_BUILD),)
    ifneq ($(strip $(wildcard $(PARENT_OF_BUILD)/src/.)),)
        BASEDIR := $(PARENT_OF_BUILD)
    endif
endif
export BASEDIR
# -----------------------------------------------------------------------------
# checks:
# -----------------------------------------------------------------------------
SRC_DIR := $(BASEDIR)/src
BUI_DIR := $(BASEDIR)/build

COR_DIR := $(SRC_DIR)/kernel
HEX_DIR := $(SRC_DIR)/hexfnt

OBJ_DIR := $(BUI_DIR)/obj
BIN_DIR := $(BUI_DIR)/bin
HEX_OUT := $(BUI_DIR)/hex

README_FILE := $(firstword $(wildcard $(BASEDIR)/README.md))
# -----------------------------------------------------------------------------
# src directory must exists ...
# -----------------------------------------------------------------------------
ifeq ($(strip $(wildcard $(SRC_DIR)/.)),)
    $(error src directory not found: $(SRC_DIR))
endif
# -----------------------------------------------------------------------------
# README.md optional (Warnung)
# -----------------------------------------------------------------------------
ifeq ($(strip $(README_FILE)),)
    $(warning no README.md found in: $(BASEDIR))
endif
# -----------------------------------------------------------------------------
# cross compiler in action ?
# -----------------------------------------------------------------------------
CROSS    ?=
EXT      :=

# TODO
GCC_DIR  := /mingw32/bin
BLD_DIR  := /usr/bin

ifeq ($(HOST_OS),windows)
EXT      += .exe
endif

# -----------------------------------------------------------------------------
# GNU Toolchain binary avatar's ...
# -----------------------------------------------------------------------------
MAKE     := $(BLD_DIR)/make$(EXT)
AS       := $(BLD_DIR)/nasm$(EXT)
ECHO     := $(BLD_DIR)/echo$(EXT)
EXPORT   := $(BLD_DIR)/export$(EXT)
GZIP     := $(BLD_DIR)/gzip$(EXT)
RM       := $(BLD_DIR)/rm$(EXT)
MKDIR    := $(BLD_DIR)/mkdir$(EXT)
STRIP    := $(BLD_DIR)/strip$(EXT)
COPY     := $(BLD_DIR)/cp$(EXT)

GCC      := $(GCC_DIR)/$(CROSS)gcc$(EXT)
CPP      := $(GCC_DIR)/$(CROSS)g++$(EXT)

LD       := $(GCC_DIR)/$(CROSS)ld$(EXT)
OBJCOPY  := $(GCC_DIR)/$(CROSS)objcopy$(EXT)

# -----------------------------------------------------------------------------
# if you start "make" without arguments, then .PHONY does a round robin and
# priotitialized compile setip. First of all, you can check the gattered infos
# on your computer display device.
# With start of the compilation race, you agreed the license terms in file:
# LICENSE in the root directory of this reporsitory.
# -----------------------------------------------------------------------------
.PHONY: confirm
confirm:
	@bash -lc '\
        $(ECHO) "ATTENTION" ;\
        $(ECHO) "----"      ;\
        $(ECHO) "With start of the compilation race, you agreed the license terms in file:" ;\
        $(ECHO) "LICENSE in the root directory of this reporsitory." ;\
        $(ECHO) "";\
        $(ECHO) "Collect Informations done."   ;\
        $(ECHO) "----"                         ;\
        $(ECHO) "CURDIR     = $(CURDIR_ABS)"   ;\
        $(ECHO) "BASEDIR    = $(BASEDIR)"      ;\
        $(ECHO) "SRC_DIR    = $(SRC_DIR)"      ;\
        $(ECHO) "HEX_DIR    = $(HEX_DIR)"      ;\
        $(ECHO) "COR_DIR    = $(COR_DIR)"      ;\
        $(ECHO) "BUI_DIR    = $(BUI_DIR)"      ;\
        $(ECHO) "BIN_DIR    = $(BIN_DIR)"      ;\
        $(ECHO) "OBJ_DIR    = $(OBJ_DIR)"      ;\
        $(ECHO) "HEX_OUT    = $(HEX_OUT)"      ;\
        $(ECHO) "README     = $(if $(README_FILE),$(README_FILE),<not found>)";\
        $(ECHO) "----" ;\
        $(ECHO) "build date = $(DATE_YMD)" ;\
        $(ECHO) "build time = $(TIME_HMS)" ;\
        $(ECHO) "----" ;\
        $(ECHO) "Accept the license ? [Y / J / N]"  ;\
        if [[ "$(AUTO_YES)" == "1" || -n "$$CI" || ! -t 0 ]]; then \
            $(ECHO) "auto-yes";\
            exit 0;\
        fi; \
        read -r ans ;\
        case "$$ans" in \
            [Nn])   $(ECHO) "CANCELED."       ; exit 1 ;; \
            [YyJj])                             exit 0 ;; \
            *) $(ECHO) "Please press Y or N." ; exit 1 ;; \
        esac \
	'
	@(  $(ECHO) "setup environment ..." ;\
        mkdir -p $(HEX_DIR) ;\
        mkdir -p $(BUI_DIR)/{bin,hex,obj/coff,obj/elf} ;\
        if [ -f "$(HEX_OUT)/font8x16.h" ]; then \
            echo "$(HEX_OUT)/font8x16.h: found."; \
        else \
            echo "Update: font8x16.h" ; \
            cd $(HEX_DIR) ;\
            wget --no-check-certificate \
            https://mirrors.kernel.org/gnu/unifont/unifont-15.1.05/unifont_all-15.1.05.hex.gz ;\
            gunzip unifont_all-15.1.05.hex.gz ;\
            mv unifont_all-15.1.05.hex $(HEX_OUT)/unifont8x16.hex ;\
            cd $(HEX_OUT) ;\
            $(HEX_DIR)/make_unifont8x16.py \
            $(HEX_OUT)/unifont8x16.hex     \
            $(HEX_OUT)/font8x16.h ;\
        fi ;\
	)

# -----------------------------------------------------------------------------
# compiler flags ...
# -----------------------------------------------------------------------------
CFLAGS   := -m32 -O1 -ffreestanding -fno-stack-protector -fno-builtin   \
            -nostdlib -nostartfiles -fno-pic -Wall -Wextra              \
            -mno-ms-bitfields -Wno-unused-variable                      \
            -Wno-unused-value -Wno-unused-parameter -Wno-sign-compare   \
            -Wno-missing-field-initializers                             \
            -D__BUILD_DATE__=\"$(DATE_YMD)\" \
            -D__BUILD_TIME__=\"$(TIME_HMS)\" \
            -I$(SRC_DIR)/kernel/include

CPPFLAGS := -std=c++20 -Wno-write-strings -Wno-volatile

LDFLAGS  := -nostdlib -T $(COR_DIR)/kernel.ld -Map ../bin/kernel.map
LDSHFLGS := -nostdlib -T $(COR_DIR)/shell.ld  -Map ../bin/shell.map

ASMFLAGS := -f win32 -O2 -DDOS_MODE=32
ISOGUI   ?= 0

# -----------------------------------------------------------------------------
# we support 2 modes: text and/or gui ...
# -----------------------------------------------------------------------------
ifeq ($(ISOGUI),1)
ASMFLAGS += -DISOGUI=1
CFLAGS   += -DISOGUI=1
CPPFLAGS += -DISOGUI=1
else
ASMFLAGS += -DISOGUI=0
CFLAGS   += -DISOGUI=0
CPPFLAGS += -DISOGUI=0
endif

# -----------------------------------------------------------------------------
# input/output directories to hold the source tree clean ...
# -----------------------------------------------------------------------------
BASE := 
SRC  := $(BASEDIR)/src
OBJ  := $(BASEDIR)/build/obj
BIN  := $(BASEDIR)/build/bin

# -----------------------------------------------------------------------------
# source files for the C-kernel ...
# -----------------------------------------------------------------------------
SRC_SHELL       :=\
        $(SRC)/user32/shell32/shell_loader.cc  \
        $(SRC)/user32/shell32/shell.cc

SRC_FS_ISO9660  :=\
        $(COR_DIR)/fs/iso9660/iso9660.c

SRC_MATH        :=\
        $(COR_DIR)/math.c

SRC_VIDEO       :=\
        $(COR_DIR)/video.c          \
        $(COR_DIR)/vga.cc           \
        $(COR_DIR)/bitmap.cc

SRCC := $(COR_DIR)/ckernel.cc       \
        $(COR_DIR)/paging.c         \
        $(COR_DIR)/idt.c            \
        $(COR_DIR)/irqc.c           \
        $(COR_DIR)/isrc.c           \
        $(COR_DIR)/gdt.c            \
        $(COR_DIR)/syscall.c        \
        $(COR_DIR)/timer.c          \
        $(COR_DIR)/usermodec.c      \
        $(COR_DIR)/task.c           \
        \
        $(SRC_MATH)         \
        $(SRC_FS_ISO9660)   \
        $(SRC_VIDEO)        \
        \
        $(COR_DIR)/util.c           \
        $(COR_DIR)/atapi.c          \
        $(COR_DIR)/ahci.c           \
        $(COR_DIR)/ps2_mouse.cc     \
        $(COR_DIR)/int86.c          \
        $(COR_DIR)/pe.c             \
        $(COR_DIR)/wm.c             \
        $(COR_DIR)/kheap.cc         \
        \
        $(SRC_SHELL)
# -----------------------------------------------------------------------------
# source files for the C-kernel ...
# -----------------------------------------------------------------------------
SHSRCC := \
        $(SRC)/shell.c          \
        $(SRC)/ps2_mouse.cc     \
        $(SRC)/paging.c

# -----------------------------------------------------------------------------
# *.o bject files for linkage stage of the C-kernel ...
# -----------------------------------------------------------------------------
OBJS := $(OBJ)/coff/ckernel.o        \
        $(OBJ)/coff/paging.o         \
        $(OBJ)/coff/flush.o          \
        $(OBJ)/coff/idt.o            \
        $(OBJ)/coff/isr.o            \
        $(OBJ)/coff/irqc.o           \
        $(OBJ)/coff/irq.o            \
        $(OBJ)/coff/isrc.o           \
        $(OBJ)/coff/gdt.o            \
        $(OBJ)/coff/syscall.o        \
        $(OBJ)/coff/timer.o          \
        $(OBJ)/coff/math.o           \
        $(OBJ)/coff/usermode.o       \
        $(OBJ)/coff/usermodec.o      \
        $(OBJ)/coff/task.o           \
        $(OBJ)/coff/iso9660.o        \
        $(OBJ)/coff/video.o          \
        $(OBJ)/coff/vga.o            \
        $(OBJ)/coff/bitmap.o         \
        $(OBJ)/coff/util.o           \
        $(OBJ)/coff/atapi.o          \
        $(OBJ)/coff/ahci.o           \
        $(OBJ)/coff/ps2_mouse.o      \
        $(OBJ)/coff/shell_loader.o   \
        $(OBJ)/coff/shell.o          \
        $(OBJ)/coff/int86_blob.o     \
        $(OBJ)/coff/int86.o          \
        $(OBJ)/coff/pe.o             \
        $(OBJ)/coff/wm.o             \
        $(OBJ)/coff/kheap.o
# -----------------------------------------------------------------------------
# *.o bject files for linkage stage of the shell ...
# -----------------------------------------------------------------------------
SHOBJS := \
        $(OBJ)/coff/shell.o          \
        $(OBJ)/coff/kheap.o          \
        $(OBJ)/coff/math.o           \
        $(OBJ)/coff/paging.o         \
        $(OBJ)/coff/ps2_mouse.o      \
        $(OBJ)/coff/vga.o            \
        $(OBJ)/coff/video.o          \
        $(OBJ)/coff/util.o

LBA        := $(COR_DIR)/lba.inc
ISO        := $(BIN_DIR)/bootcd.iso
ISO_FILES  := /boot2.bin /kernel.bin

all: $(OBJ)/coff/data.o
#$(BIN)/bootcd.iso

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
$(BIN)/boot1.bin: $(SRC)/kernel/boot/boot1.asm
	$(AS) -f bin  $< -o $@
$(BIN)/boot2.bin: $(SRC)/kernel/boot/boot2.asm
	$(AS) -f bin  $< -o $@
# -----------------------------------------------------------------------------
# compile all *.c files to *.o bject files ...
# -----------------------------------------------------------------------------
$(OBJ)/coff/%.o: $(SRC)/kernel/%.c
	$(GCC) $(CFLAGS) -c $< -o $@
$(OBJ)/coff/%.o: $(SRC)/kernel/fs/iso9660/%.c
	$(GCC) $(CFLAGS) -c $< -o $@

$(OBJ)/coff/%.o: $(SRC)/kernel/%.cc
	$(CPP) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# -----------------------------------------------------------------------------
# compile all *.asm files to *.o bject files ...
# -----------------------------------------------------------------------------
$(OBJ)/coff/%.o: $(SRC)/kernel/%.asm
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

$(BIN)/bootcd.iso: $(BIN)/boot1.bin $(BIN)/boot2.bin \
    $(BIN)/int86_blob.bin \
    $(BIN)/kernel.bin     \
    $(BIN)/shell.exe
	./create_iso.sh
	$(MOD-LBA)

$(BIN)/int86_blob.bin: $(SRC)/int86_switch.asm
	nasm -f bin -o $@ $<

# -----------------------------------------------------------------------------
# prepare font ...
# -----------------------------------------------------------------------------
$(HEX_DIR)/unifont.hex:        \
    $(HEX_DIR)/unifont8x16.hex \
    $(SRC_DIR)/make_unifont8x16.py
	python3 $(SRC)/make_unifont8x16.py unifont8x16.hex font8x16.h

# -----------------------------------------------------------------------------
# before you compile, you can clean the source tree from old crap ...
# -----------------------------------------------------------------------------
clean:
	$(RM) -rf $(OBJ_DIR)

#	$(RM) -f  $(BIN)/*.BIN $(BIN)/*.COM $(BIN)/*.sys $(BIN)/kernel.map
#	$(RM) -f  $(SRC)/content/*.sys $(SRC)/content/*.bin
#	$(RM) -f  $(SRC)/unifont_all-15.1.05.hex
#	$(RM) -f  $(BIN)/bootcd.iso
#	$(RM) -f  $(SRC)/unifont.hex
#	$(RM) -f  $(SRC)/initrd.dat
#	$(RM) -f  $(BIN)/hdd.img $(BIN)/shell.exe
#	$(RM) -rf $(OBJ)
