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
MAKE     := $(shell command which make   )$(EXT)
AS       := $(shell command which as     )$(EXT)
ECHO     := $(shell command which echo   )$(EXT)
GZIP     := $(shell command which gzip   )$(EXT)
RM       := $(shell command which rm     )$(EXT)
MKDIR    := $(shell command which mkdir  )$(EXT)
STRIP    := $(shell command which strip  )$(EXT)
COPY     := $(shell command which cp     )$(EXT)

XORRISO  := $(shell command which xorriso)$(EXT)
SED      := $(shell command which sed    )$(EXT)

MFORMAT  := $(shell command which mformat)$(EXT)
MCOPY    := $(shell command which mcopy  )$(EXT)

GCC      := $(shell command which gcc    )$(EXT)
CPP      := $(shell command which g++    )$(EXT)

LD       := $(shell command which ld     )$(EXT)
OBJCOPY  := $(shell command which objcopy)$(EXT)

# -----------------------------------------------------------------------------
# if you start "make" without arguments, then .PHONY does a round robin and
# priotitialized compile setip. First of all, you can check the gattered infos
# on your computer display device.
# With start of the compilation race, you agreed the license terms in file:
# LICENSE in the root directory of this reporsitory.
# -----------------------------------------------------------------------------
#.PHONY: confirm
all: confirm clean setup shell32 $(BIN_DIR)/bootcd.iso
setup:
	$(MKDIR) -p $(BUI_DIR) $(BUI_DIR)/bin $(BUI_DIR)/bin/content $(BUI_DIR)/hex
	$(MKDIR) -p $(BIN_DIR)/content/img $(BUI_DIR)/obj $(BUI_DIR)/obj/pe
	$(MKDIR) -p $(OBJ_DIR)/user32/TurboVision/platform
	$(MKDIR) -p $(OBJ_DIR)/user32/TurboVision
	$(MKDIR) -p $(OBJ_DIR)/user32/rtl
	$(MKDIR) -p $(OBJ_DIR)/user32/shell32
	$(MKDIR) -p $(OBJ_DIR)/stl
	
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
            -nostdlib -fno-ident  \
            -nostartfiles         \
            -nostdinc             \
            -fno-stack-protector  \
            -fno-unwind-tables    \
            -fno-asynchronous-unwind-tables \
            -fno-builtin          \
            -fno-pic              \
            -Wno-uninitialized    \
            -Wno-maybe-uninitialized \
            -mno-ms-bitfields     \
            -Wno-unused-variable  \
            -Wno-unused-function  \
            -Wno-unused-parameter \
            -Wno-unused-value     \
            -Wno-sign-compare     \
            -Wno-unused-but-set-parameter    \
            -Wno-missing-field-initializers  \
            -D__BUILD_DATE__=\"$(DATE_YMD)\" \
            -D__BUILD_TIME__=\"$(TIME_HMS)\" \
            -I$(SRC_DIR)/kernel/include      \
            -I$(SRC_DIR)                \
            -I$(SRC_DIR)/stl/inc        \
            -I$(SRC_DIR)/stl/inc/string \
            -I$(SRC_DIR)/user32                  \
            -I$(SRC_DIR)/user32/include          \
            -I$(SRC_DIR)/user32/TurboVision      \
            -I$(SRC_DIR)/user32/TurboVision/inc  \
            -I$(SRC_DIR)/fntres

CFLAGS_CC:= -std=c++20  $(CFLAGS_C) \
            -nostdinc++             \
            -fno-exceptions         \
            -fno-rtti               \
            -fno-use-cxa-atexit     \
            -Wno-write-strings      \
            -Wno-volatile           \
            -fno-use-cxa-atexit     \
            -fno-threadsafe-statics

LDFLAGS  := -nostdlib -T $(COR_DIR)/kernel.ld -Map $(BIN_DIR)/kernel.map
LDSHFLGS := -nostdlib -T $(U32_DIR)/shell32/shell32.ld -Map $(BIN_DIR)/shell32.map

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

KERNEL_PASS  := 0
SHELL32_PASS := 1
STL32_PASS   := 2

# -----------------------------------------------------------------------------
# source files for the C-kernel ...
# -----------------------------------------------------------------------------
SRC_SHELL       :=\
        $(SRC)/user32/shell32/shell32.cc    \
        $(SRC)/user32/rtl/rtl_ExitProcess.c

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
        $(OBJ_DIR)/coff/int86_blob.o     \
        $(OBJ_DIR)/coff/int86.o          \
        $(OBJ_DIR)/coff/pe.o             \
        $(OBJ_DIR)/coff/wm.o             \
        $(OBJ_DIR)/coff/kheap.o          \
        \
        $(OBJ_DIR)/coff/elf_loader.o     \
        $(OBJ_DIR)/coff/pe_loader.o      \
        \
        $(OBJ_DIR)/coff/clear_screen.o   \
        $(OBJ_DIR)/coff/ksymbol_table.o  \
        \
        $(RUN_CPO) \
        $(OBJ_DIR)/coff/kstl.o \
        \
        $(OBJ_DIR)/coff/roboto12x16.o   \
        $(OBJ_DIR)/coff/testfont.o      \
        $(OBJ_DIR)/coff/petschii.o

# -----------------------------------------------------------------------------
# *.o bject files for linkage stage of the shell ...
# -----------------------------------------------------------------------------
SHOBJS := \
        $(OBJ_DIR)/user32/shell32/shell32.o              \
        $(OBJ_DIR)/user32/shell32/no_rtti.o              \
        $(OBJ_DIR)/user32/shell32/shell32_app.o          \
        $(OBJ_DIR)/user32/symbol_table.o                 \
        $(OBJ_DIR)/user32/TurboVision/Application.o      \
        $(OBJ_DIR)/user32/TurboVision/TObject.o          \
        $(OBJ_DIR)/user32/TurboVision/TDesktop.o         \
        $(OBJ_DIR)/user32/TurboVision/TMenuBar.o         \
        $(OBJ_DIR)/user32/TurboVision/TStatusBar.o       \
        $(OBJ_DIR)/user32/TurboVision/THardware.o        \
        $(OBJ_DIR)/user32/TurboVision/TVideo.o           \
        $(OBJ_DIR)/user32/TurboVision/rtc.o              \
        $(OBJ_DIR)/user32/TurboVision/util.o             \
        $(OBJ_DIR)/user32/TurboVision/platform/strings.o \
        $(OBJ_DIR)/user32/rtl/rtl_ExitProcess.o          \
        $(OBJ_DIR)/stl/stl_init.o

ISO_FILES  := /boot2.bin /kernel.bin
LBA        := $(COR_DIR)/lba.inc
ISO        := $(BIN_DIR)/bootcd.iso

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
                print name "_LBA     equ ", $$6, "   ; start LBA ", name; \
                print name "_SECTORS equ ", $$8, "   ; 2048 * "   , $$8 ; \
                print "; -----------------------------------------------------"; \
            }   }'  >> $(LBA); \
        done \
	)    
	@echo "VBE_INFO_ADDR   equ 0x0A00" >> $(LBA)
    
	nasm -DLBA_FILE=\"$(COR_DIR)/lba.inc\"  \
        -f bin $(COR_DIR)/boot/boot1.asm    \
        -o     $(BIN_DIR)/content/boot1.bin \
        -l     $(BIN_DIR)/boot1.lst
    
	nasm -DLBA_FILE=\"$(COR_DIR)/lba.inc\"  \
        -f bin $(COR_DIR)/boot/boot2.asm    \
        -o     $(BIN_DIR)/content/boot2.bin \
        -l     $(BIN_DIR)/boot2.lst
    
	$(XORRISO) -as mkisofs -o $(BIN_DIR)/bootcd.iso  \
        -b boot1.bin          \
        -no-emul-boot         \
        -boot-load-size 4     \
        -V "BOOTCD" $(BIN_DIR)/content
    cd ..
    @echo "DONE"
endef
# -----------------------------------------------------------------------------
# loader to load the system from DOS console ...
# -----------------------------------------------------------------------------
$(BIN_DIR)/content/boot1.bin:  $(COR_DIR)/boot/boot1.asm
	nasm -DLBA_FILE=\"$(COR_DIR)/lba.inc\" -f bin  $< -o $@
$(BIN_DIR)/content/boot2.bin:  $(COR_DIR)/boot/boot2.asm
	nasm -DLBA_FILE=\"$(COR_DIR)/lba.inc\" -f bin  $< -o $@
# -----------------------------------------------------------------------------
# compile all *.c, *.cc, and *.asm files to *.o bject files ...
# -----------------------------------------------------------------------------
define compile_rule
$(2)/%.s: $(3)/%.c
	$(MKDIR) -p $(dir $$@) $(DEP_DIR)/coff/$(dir $$*)
	$(GCC) -m32 $(CFLAGS_C) -MMD -MP \
		-MF $(DEP_DIR)/coff/$$*.d -MT $$@ \
		-S $$< -o $$@
	@if [ "$(1)" = "$(SHELL32_PASS)" ] || [ "$(1)" = "$(STL32_PASS)" ]; then \
        $(SED) -i "/^[[:space:]]*\.ident[[:space:]]*\"GCC:/d" $$@; \
	fi

$(2)/%.s: $(3)/%.cc
	$(MKDIR) -p $(dir $$@) $(DEP_DIR)/coff/$(dir $$*)
	$(CPP) -m32 $(CFLAGS_CC) -MMD -MP \
		-MF $(DEP_DIR)/coff/$$*.d -MT $$@ \
		-S $$< -o $$@
	@if [ "$(1)" = "$(SHELL32_PASS)" ] || [ "$(1)" = "$(STL32_PASS)" ]; then \
        $(SED) -i "/^[[:space:]]*\.ident[[:space:]]*\"GCC:/d" $$@; \
	fi

$(2)/%.o: $(3)/%.s
	$(GCC) -m32 $(CFLAGS_C) -c $$< -o $$(patsubst %.s,%.o,$$@)

$(2)/%.o: $(3)/%.asm
	$(MKDIR) -p $(dir $$@)
	nasm $(ASMFLAGS) $$< -o $$@
endef

$(eval $(call compile_rule,$(KERNEL_PASS),$(OBJ_DIR)/coff,$(COR_DIR)))
$(eval $(call compile_rule,$(KERNEL_PASS),$(OBJ_DIR)/coff,$(COR_DIR)/fs/iso9660))
$(eval $(call compile_rule,$(KERNEL_PASS),$(OBJ_DIR)/coff,$(COR_DIR)/video))

# -----------------------------------------------------------------------------
# compile rule for program loaders ...
# -----------------------------------------------------------------------------
$(eval $(call compile_rule,$(KERNEL_PASS),$(OBJ_DIR)/coff,$(COR_DIR)/loader/elf))
$(eval $(call compile_rule,$(KERNEL_PASS),$(OBJ_DIR)/coff,$(COR_DIR)/loader/pe))

$(eval $(call compile_rule,$(KERNEL_PASS),$(OBJ_DIR)/coff,$(SRC_DIR)/fntres))

# -----------------------------------------------------------------------------
# static RTL - run time library ...
# -----------------------------------------------------------------------------
$(eval $(call compile_rule,$(STL32_PASS),$(OBJ_DIR)/stl,$(SRC_DIR)/stl/src))

# -----------------------------------------------------------------------------
# create TurboVision (BORLAND) text user interface ...
# -----------------------------------------------------------------------------
$(eval $(call compile_rule,$(SHELL32_PASS),$(OBJ_DIR)/user32/TurboVision,$(SRC_DIR)/user32/TurboVision/src))
$(eval $(call compile_rule,$(SHELL32_PASS),$(OBJ_DIR)/user32/TurboVision/platform,$(SRC_DIR)/user32/TurboVision/src/platform))
$(eval $(call compile_rule,$(SHELL32_PASS),$(OBJ_DIR)/user32/rtl,$(SRC_DIR)/user32/rtl))

$(eval $(call compile_rule,$(SHELL32_PASS),$(OBJ_DIR)/user32,$(SRC_DIR)/user32))
$(eval $(call compile_rule,$(SHELL32_PASS),$(OBJ_DIR)/user32/shell32,$(SRC_DIR)/user32/shell32))

DEPS := $(patsubst $(OBJ_DIR)/coff/%.o,$(DEP_DIR)/coff/%.d,$(OBJS))
-include $(DEPS)

# -----------------------------------------------------------------------------
# link C-kernel to finaly output binary image ...
# -----------------------------------------------------------------------------
$(OBJ_DIR)/coff/kernel.o: $(OBJS) $(COR_DIR)/kernel.ld
	$(CPP) -nostdlib -nodefaultlibs -nostartfiles \
	  -Wl,-T,$(COR_DIR)/kernel.ld -Wl,-Map,$(BIN_DIR)/kernel.map \
	  -o $@ $(OBJS)

$(OBJ_DIR)/kernel.bin:  $(OBJ_DIR)/coff/kernel.o
	$(OBJCOPY) -O binary $< $@
# -----------------------------------------------------------------------------
# create PE32 shell32 application
# -----------------------------------------------------------------------------
$(BIN_DIR)/content/shell32.exe: $(SHOBJS) \
    $(SRC_DIR)/user32/shell32/shell32.ld
	$(GCC) -m32 -mconsole $(CFLAGS_C) -o $(BIN_DIR)/content/shell32.exe $(SHOBJS)

shell32:  $(BIN_DIR)/content/shell32.exe
	strip $(BIN_DIR)/content/shell32.exe
	objcopy --remove-section .eh_frame     \
            --remove-section .eh_frame_hdr \
            --remove-section .gcc_except_table \
            $(BIN_DIR)/content/shell32.exe \
            $(BIN_DIR)/content/shell32.exe
	echo "dodo"

$(BIN_DIR)/bootcd.iso: \
    $(BIN_DIR)/content/boot1.bin $(BIN_DIR)/content/boot2.bin \
    $(OBJ_DIR)/coff/int86_blob.o \
    $(OBJ_DIR)/kernel.bin
	@(  export __BIN_DIR=$(BIN_DIR)          ;\
        export __COR_DIR=$(COR_DIR)          ;\
        export __OBJ_DIR=$(OBJ_DIR)/coff     ;\
        export __BIN_AS=nasm                 ;\
        $(COR_DIR)/create_iso.sh             ;\
	)
	$(MOD-LBA)

$(OBJ_DIR)/coff/int86_switch.bin: $(COR_DIR)/int86_switch.asm
	nasm -f bin -o $(OBJ_DIR)/coff/int86_switch.bin $(COR_DIR)/int86_switch.asm
$(OBJ_DIR)/coff/int86_blob.o: $(OBJ_DIR)/coff/int86_switch.bin
	nasm -DSWITCH_BLOB=\"$(OBJ_DIR)/coff/int86_switch.bin\" -f win32 -o $(OBJ_DIR)/coff/int86_blob.o $(COR_DIR)/int86_blob.asm
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
	/mingw64/bin/qemu-system-i386.exe \
        -drive file=$(BIN_DIR)/bootcd.iso,if=none,media=cdrom,id=cdrom0 \
        -machine q35,i8042=on \
        -device ich9-ahci,id=ahci0 \
        -device ide-cd,drive=cdrom0,bus=ahci0.0 \
        -boot d -m 512M
