# -----------------------------------------------------------------------------
# \file  Makefile
# \note  (c) 2025 by Jens Kallup - paule32
#        all rights reserved.
# -----------------------------------------------------------------------------
# we only support MinGW 32-bit and Linux Systems ...
# -----------------------------------------------------------------------------
CYGPATH := cygpath

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
DEP_DIR := $(BASEDIR)/build/dep
BUI_DIR := $(BASEDIR)/build

COR_DIR := $(SRC_DIR)/kernel
HEX_DIR := $(SRC_DIR)/hexfnt
IMG_DIR := $(SRC_DIR)/images

U32_DIR := $(SRC_DIR)/user32
A32_DIR := $(SRC_DIR)/admin0

COB_DIR := $(BUI_DIR)/obj/coff
OBJ_DIR := $(BUI_DIR)/obj
BIN_DIR := $(BUI_DIR)/bin
HEX_OUT := $(BUI_DIR)/hex

FNT_DIR := $(SRC_DIR)/fntres

MBR_BIN := $(BIN_DIR)/fat32_mbr.bin
VBR_BIN := $(BIN_DIR)/fat32_vbr.bin

README_FILE := $(firstword $(wildcard $(BASEDIR)/README.md))
# -----------------------------------------------------------------------------
# src directory must exists ...
# -----------------------------------------------------------------------------
ifeq ($(strip $(wildcard $(SRC_DIR))),)
    $(error src directory not found: $(SRC_DIR))
endif
# -----------------------------------------------------------------------------
# README.md optional (Warnung)
# -----------------------------------------------------------------------------
ifeq ($(strip ../$(README_FILE)),)
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

ISO_IMG  := $(BIN_DIR)/bootcd.iso
USB_IMG  := $(BIN_DIR)/bootusb.iso

ifeq ($(HOST_OS),windows)
EXT      += .exe
endif

# -----------------------------------------------------------------------------
# GNU Toolchain binary avatar's ...
# -----------------------------------------------------------------------------
MAKE     := $(BLD_DIR)/make$(EXT)
AS       := nasm
ECHO     := $(BLD_DIR)/echo$(EXT)
EXPORT   := $(BLD_DIR)/export$(EXT)
GZIP     := $(BLD_DIR)/gzip$(EXT)
RM       := $(BLD_DIR)/rm$(EXT)
MKDIR    := $(BLD_DIR)/mkdir$(EXT)
STRIP    := $(BLD_DIR)/strip$(EXT)
COPY     := $(BLD_DIR)/cp$(EXT)

XORRISO  := $(BLD_DIR)/xorriso$(EXT)
MFORMAT  := $(GCC_DIR)/mformat$(EXT)
MCOPY    := $(GCC_DIR)/mcopy$(EXT)

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
#.PHONY: confirm
all: confirm clean setup $(BIN_DIR)/bootcd.iso
setup:
	$(MKDIR) -p $(BUI_DIR) $(BUI_DIR)/bin $(BUI_DIR)/bin/content $(BUI_DIR)/hex
	$(MKDIR) -p $(BIN_DIR)/content/img
	$(COPY) $(IMG_DIR)/*.bmp $(BIN_DIR)/content/img
	(   cd $(SRC_DIR)/fntres        ;\
        $(SRC_DIR)/fntres/build.sh  ;\
        cd $(BASEDIR)               \
	)
	$(GEN-LBA)
confirm:
	@(  $(ECHO) "ATTENTION" ;\
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
	)
	@(  $(ECHO) "setup environment ..."   ;\
        $(MKDIR) -p $(BUI_DIR) $(BUI_DIR)/bin $(BUI_DIR)/bin/content $(BUI_DIR)/hex ;\
        $(MKDIR) -p $(BIN_DIR)/content/img ;\
        $(MKDIR) -p $(HEX_DIR)           ;\
        if [ -f "$(HEX_OUT)/font8x16.h" ]; then  \
            echo "$(HEX_OUT)/font8x16.h: found.";\
            exit 0 ;\
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
CFLAGS_C := -m32 -O1 -ffreestanding -Wall -Wextra \
            -nostdlib             \
            -nostartfiles         \
            -fno-stack-protector  \
            -fno-builtin          \
            -fno-pic              \
            -mno-ms-bitfields     \
            -Wno-unused-variable  \
            -Wno-unused-function  \
            -Wno-unused-parameter \
            -Wno-unused-value     \
            -Wno-sign-compare     \
            -Wno-missing-field-initializers  \
            -D__BUILD_DATE__=\"$(DATE_YMD)\" \
            -D__BUILD_TIME__=\"$(TIME_HMS)\" \
            -I$(SRC_DIR)/kernel/include \
            -I$(SRC_DIR)/fntres

CFLAGS_CC:= -std=c++20  $(CFLAGS_C) \
            -fexceptions -frtti     \
            -Wno-write-strings      \
            -Wno-volatile           \
            -fno-use-cxa-atexit     \
            -fno-threadsafe-statics

LDFLAGS  := -nostdlib -T $(COR_DIR)/kernel.ld -Map $(BIN_DIR)/kernel.map
LDSHFLGS := -nostdlib -T $(U32_DIR)/shell32/shell.ld -Map $(BIN_DIR)/shell.map

ASMFLAGS := -f win32 -O2 -DDOS_MODE=32
ISOGUI   ?= 0

# -----------------------------------------------------------------------------
# we support 2 modes: text and/or gui ...
# -----------------------------------------------------------------------------
ifeq ($(ISOGUI),1)
ASMFLAGS  += -DISOGUI=1
CFLAGS_C  += -DISOGUI=1
CFLAGS_CC += -DISOGUI=1
else
ASMFLAGS  += -DISOGUI=0
CFLAGS_C  += -DISOGUI=0
CFLAGS_CC += -DISOGUI=0
endif

# -----------------------------------------------------------------------------
# input/output directories to hold the source tree clean ...
# -----------------------------------------------------------------------------
BASE := 
SRC  := $(BASEDIR)/src
OBJ  := $(BASEDIR)/build/obj
BIN  := $(BASEDIR)/build/bin

PART_START_SECTOR   := 2048
USB_SIZE_MIB        := 64

# -----------------------------------------------------------------------------
# source files for the C-kernel ...
# -----------------------------------------------------------------------------
SRC_SHELL       :=\
        $(SRC)/user32/shell32/shell_loader.cc  \
        $(SRC)/user32/shell32/shell.cc

RUN_CPO := \
        $(OBJ_DIR)/coff/cpp_runtime.o \
        $(OBJ_DIR)/coff/ctor.o

SRC_FONTS := \
        $(SRC_DIR)/fntres/roboto12x16.cc \
        $(SRC_DIR)/fntres/testfont.cc

SRC_FS_ISO9660  :=\
        $(COR_DIR)/fs/iso9660/iso9660.c

SRC_MATH        :=\
        $(COR_DIR)/math.c
        
SRC_KXSTL := $(COR_DIR)kstl.cc

SRC_CPPRT       :=\
        $(COR_DIR)/cpp_runtime.o \
        $(COR_DIR)/ctors.cc

SRC_VIDEO       :=\
        $(COR_DIR)/video.c          \
        $(COR_DIR)/video/vga.cc     \
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
        $(COR_DIR)/wm.cc            \
        $(COR_DIR)/kheap.cc         \
        \
        $(SRC_FONTS) \
        $(SRC_KXSTL) \
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
OBJS := $(OBJ_DIR)/coff/ckernel.o        \
        $(OBJ_DIR)/coff/paging.o         \
        $(OBJ_DIR)/coff/flush.o          \
        $(OBJ_DIR)/coff/idt.o            \
        $(OBJ_DIR)/coff/isr.o            \
        $(OBJ_DIR)/coff/irqc.o           \
        $(OBJ_DIR)/coff/irq.o            \
        $(OBJ_DIR)/coff/isrc.o           \
        $(OBJ_DIR)/coff/gdt.o            \
        $(OBJ_DIR)/coff/syscall.o        \
        $(OBJ_DIR)/coff/timer.o          \
        $(OBJ_DIR)/coff/math.o           \
        $(OBJ_DIR)/coff/usermode.o       \
        $(OBJ_DIR)/coff/usermodec.o      \
        $(OBJ_DIR)/coff/task.o           \
        $(OBJ_DIR)/coff/iso9660.o        \
        $(OBJ_DIR)/coff/video.o          \
        $(OBJ_DIR)/coff/vga.o            \
        $(OBJ_DIR)/coff/bitmap.o         \
        $(OBJ_DIR)/coff/util.o           \
        $(OBJ_DIR)/coff/atapi.o          \
        $(OBJ_DIR)/coff/ahci.o           \
        $(OBJ_DIR)/coff/ps2_mouse.o      \
        $(OBJ_DIR)/coff/shell_loader.o   \
        $(OBJ_DIR)/coff/shell.o          \
        $(OBJ_DIR)/coff/int86_blob.o     \
        $(OBJ_DIR)/coff/int86.o          \
        $(OBJ_DIR)/coff/pe.o             \
        $(OBJ_DIR)/coff/wm.o             \
        $(OBJ_DIR)/coff/kheap.o          \
        \
        $(RUN_CPO) \
        $(OBJ_DIR)/coff/kstl.o \
        \
        $(OBJ_DIR)/coff/roboto12x16.o   \
        $(OBJ_DIR)/coff/testfont.o

# -----------------------------------------------------------------------------
# *.o bject files for linkage stage of the shell ...
# -----------------------------------------------------------------------------
SHOBJS := \
        $(OBJ_DIR)/coff/shell.o          \
        $(OBJ_DIR)/coff/kheap.o          \
        $(OBJ_DIR)/coff/math.o           \
        $(OBJ_DIR)/coff/paging.o         \
        $(OBJ_DIR)/coff/ps2_mouse.o      \
        $(OBJ_DIR)/coff/vga.o            \
        $(OBJ_DIR)/coff/video.o          \
        $(OBJ_DIR)/coff/util.o

ISO_FILES  := /boot2.bin /kernel.bin
LBA        := $(COR_DIR)/lba.inc
ISO        := $(BIN_DIR)/bootcd.iso

#all: $(OBJ)/coff/data.o
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
	@(  for f in $(ISO_FILES); do \
        echo "search $$f";     \
        xorriso -indev $(ISO) -find / -exec report_lba -- | \
        awk -v iso="$$f" '/^File data lba:/ {\
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
            }   }'  >> $(LBA); \
        done \
	)    
	@echo "VBE_INFO_ADDR   equ 0x0A00" >> $(LBA)
	$(AS) -DLBA_FILE=\"$(COR_DIR)/lba.inc\" -f bin $(COR_DIR)/boot/boot1.asm -o $(BIN_DIR)/content/boot1.bin
	$(AS) -DLBA_FILE=\"$(COR_DIR)/lba.inc\" -f bin $(COR_DIR)/boot/boot2.asm -o $(BIN_DIR)/content/boot2.bin
	@cd $(BIN_DIR)/content && \
	$(XORRISO) -as mkisofs -o $(BIN_DIR)/bootcd.iso  \
        -b boot1.bin       \
        -no-emul-boot      \
        -boot-load-size 4  .
    cd ..
    @echo "DONE"
endef
# -----------------------------------------------------------------------------
# loader to load the system from DOS console ...
# -----------------------------------------------------------------------------
$(BIN_DIR)/content/boot1.bin:  $(COR_DIR)/boot/boot1.asm
	$(AS) -DLBA_FILE=\"$(COR_DIR)/lba.inc\" -f bin  $< -o $@
$(BIN_DIR)/content/boot2.bin:  $(COR_DIR)/boot/boot2.asm
	$(AS) -DLBA_FILE=\"$(COR_DIR)/lba.inc\" -f bin  $< -o $@
# -----------------------------------------------------------------------------
# compile all *.c, *.cc, and *.asm files to *.o bject files ...
# -----------------------------------------------------------------------------
define compile_rule
$(OBJ_DIR)/coff/%.o: $(1)/%.c
	$(MKDIR) -p $(dir $$@) $(DEP_DIR)/coff/$(dir $$*)
	$(GCC) $(CFLAGS_C) -MMD -MP \
		-MF $(DEP_DIR)/coff/$$*.d -MT $$@ \
		-c $$< -o $$@

$(OBJ_DIR)/coff/%.o: $(1)/%.cc
	$(MKDIR) -p $(dir $$@) $(DEP_DIR)/coff/$(dir $$*)
	$(CPP) $(CFLAGS_CC) -MMD -MP \
		-MF $(DEP_DIR)/coff/$$*.d -MT $$@ \
		-c $$< -o $$@

$(OBJ_DIR)/coff/%.o: $(1)/%.asm
	$(MKDIR) -p $(dir $$@)
	$(AS) $(ASMFLAGS) $$< -o $$@
endef

$(eval $(call compile_rule,$(COR_DIR)))
$(eval $(call compile_rule,$(COR_DIR)/fs/iso9660))
$(eval $(call compile_rule,$(COR_DIR)/video))
$(eval $(call compile_rule,$(COR_DIR)/loader/elf))
$(eval $(call compile_rule,$(SRC_DIR)/fntres))

DEPS := $(patsubst $(OBJ_DIR)/coff/%.o,$(DEP_DIR)/coff/%.d,$(OBJS))
-include $(DEPS)

$(OBJ_DIR)/coff/%.o: $(SRC_DIR)/user32/shell32/%.c
	$(GCC) $(CFLAGS_C)  -c $< -o $@
$(OBJ_DIR)/coff/%.o: $(SRC_DIR)/user32/shell32/%.cc
	$(CPP) $(CFLAGS_CC) -c $< -o $@
# -----------------------------------------------------------------------------
# link C-kernel to finaly output binary image ...
# -----------------------------------------------------------------------------
$(OBJ_DIR)/coff/kernel.o: $(OBJS) $(COR_DIR)/kernel.ld
	$(CPP) -nostdlib -nodefaultlibs -nostartfiles \
	  -Wl,-T,$(COR_DIR)/kernel.ld -Wl,-Map,$(BIN_DIR)/kernel.map \
	  -o $@ $(OBJS)

#	$(LD) $(LDFLAGS) -o $(OBJ_DIR)/coff/kernel.o $(OBJS)
$(OBJ_DIR)/kernel.bin:  $(OBJ_DIR)/coff/kernel.o
	$(OBJCOPY) -O binary $< $@
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

$(OBJ)/shell.bin: $(SHOBJS) $(SRC)/shell.ld
	$(LD) $(LDSHFLGS) -o    $(OBJ)/shell.bin $(SHOBJS)

$(BIN)/shell.exe:  $(OBJ)/shell.bin
	$(OBJCOPY) -O binary $(OBJ)/shell.bin $(BIN)/shell.exe

$(BIN_DIR)/bootcd.iso: \
    $(BIN_DIR)/content/boot1.bin $(BIN_DIR)/content/boot2.bin \
    $(OBJ_DIR)/coff/int86_blob.o \
    $(OBJ_DIR)/kernel.bin
	@(  export __BIN_DIR=$(BIN_DIR)          ;\
        export __COR_DIR=$(COR_DIR)          ;\
        export __OBJ_DIR=$(OBJ_DIR)/coff     ;\
        export __BIN_AS=$(AS)                ;\
        $(COR_DIR)/create_iso.sh             ;\
	)
	$(MOD-LBA)

$(OBJ_DIR)/coff/int86_switch.bin: $(COR_DIR)/int86_switch.asm
	$(AS) -f bin -o $(OBJ_DIR)/coff/int86_switch.bin $(COR_DIR)/int86_switch.asm
$(OBJ_DIR)/coff/int86_blob.o: $(OBJ_DIR)/coff/int86_switch.bin
	$(AS) -DSWITCH_BLOB=\"$(OBJ_DIR)/coff/int86_switch.bin\" -f win32 -o $(OBJ_DIR)/coff/int86_blob.o $(COR_DIR)/int86_blob.asm
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
	$(RM)   -rf $(BUI_DIR)/bin
	$(MKDIR) -p $(BUI_DIR)/bin/content
	$(MKDIR) -p $(BUI_DIR)/{obj/coff,obj/elf,obj/inc}

# -----------------------------------------------------------------------------
# optional/bonus: QEMU boot for bootCD.iso ...
# -----------------------------------------------------------------------------
bootcd:
	/mingw64/bin/qemu-system-x86_64.exe \
    -drive file=$(BIN_DIR)/bootcd.iso,if=none,media=cdrom,id=cdrom0 \
    -machine q35,i8042=on  \
    -device ich9-ahci,id=ahci0 \
    -device ide-cd,drive=cdrom0,bus=ahci0.0 \
    -boot d -m 512M
