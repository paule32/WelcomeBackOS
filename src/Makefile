# -----------------------------------------------------------------------------
# \file  code32.asm
# \note  (c) 2025 by Jens Kallup - paule32
#        all rights reserved.
# -----------------------------------------------------------------------------
CROSS    ?= 
AS       := nasm
RM       := rm
MKDIR    := mkdir
CC       := $(CROSS)gcc
LD       := $(CROSS)ld
OBJCOPY  := $(CROSS)objcopy

# -----------------------------------------------------------------------------
# compiler flags ...
# -----------------------------------------------------------------------------
CFLAGS   := -m32 -O2 -ffreestanding -fno-builtin -nostdlib -nostartfiles \
            -Wall -Wextra -Wno-unused-value -Wno-unused-parameter

LDFLAGS  := -nostdlib -T ../src/kernel.ld
ASMFLAGS := -f win32 -O2 -DDOS_MODE=32

# -----------------------------------------------------------------------------
# input/output directories to hold the source tree clean ...
# -----------------------------------------------------------------------------
SRC  := ../src
OBJ  := ../obj
BIN  := ../bin

SRCC := $(SRC)/kernel.c $(SRC)/util.c $(SRC)/math.c $(SRC)/timer.c \
        $(SRC)/video.c
        
OBJS := $(OBJ)/kernel.o $(OBJ)/util.o $(OBJ)/math.o $(OBJ)/timer.o \
        $(OBJ)/video.o  $(OBJ)/isr.o

all: dirs $(BIN)/LOADER.COM $(BIN)/KERNEL.BIN

# -----------------------------------------------------------------------------
# loader to load the system from DOS console ...
# -----------------------------------------------------------------------------
$(BIN)/LOADER.COM: $(SRC)/loader.asm
	$(AS) -f bin $< -o $@

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.asm
	$(AS) $(ASMFLAGS) $< -o $@

$(OBJ)/kernel.coff: $(OBJS) $(SRC)/kernel.ld
	$(LD) $(LDFLAGS) -m i386pe -o $@ $(OBJS)

$(BIN)/KERNEL.BIN: $(OBJ)/kernel.coff
	$(OBJCOPY) -O binary $< $@

# -----------------------------------------------------------------------------
# create directories for output object code ...
# -----------------------------------------------------------------------------
dirs:
	$(MKDIR) -p $(BIN)
	$(MKDIR) -p $(OBJ)

# -----------------------------------------------------------------------------
# before you compile, you can clean the tree from old crap ...
# -----------------------------------------------------------------------------
clean:
	$(RM) -f *.o *.coff *.BIN *.COM
	$(RM) -rf $(BIN) $(OBJ)
