# -----------------------------------------------------------------------------
# \file  code32.asm
# \note  (c) 2025 by Jens Kallup - paule32
#        all rights reserved.
# -----------------------------------------------------------------------------
CROSS    ?= 

EXPORT   := export
AS       := ../bin/nasm
GZIP     := gzip
RM       := rm
MKDIR    := mkdir
STRIP    := strip
COPY     := cp

CC       := $(CROSS)gcc
LD       := $(CROSS)ld
OBJCOPY  := $(CROSS)objcopy

# -----------------------------------------------------------------------------
# compiler flags ...
# -----------------------------------------------------------------------------
CFLAGS   := -m32 -O2 -ffreestanding -fno-stack-protector -fno-builtin   \
            -nostdlib -nostartfiles -Wall -Wextra                       \
            -Wno-unused-value -Wno-unused-parameter -Wno-sign-compare   \
            -I../src/include

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
        $(SRC)/descriptor_table.c   \
        $(SRC)/flpydisk.c           \
        $(SRC)/fs.c                 \
        $(SRC)/gdt.c                \
        $(SRC)/idt.c                \
        $(SRC)/initrd.c             \
        $(SRC)/irq.c                \
        $(SRC)/isrs.c               \
        $(SRC)/keyboard.c           \
        $(SRC)/kheap.c              \
        $(SRC)/math.c               \
        $(SRC)/ordered_array.c      \
        $(SRC)/paging.c             \
        $(SRC)/shared_pages.c       \
        $(SRC)/syscall.c            \
        $(SRC)/task.c               \
        $(SRC)/timer.c              \
        $(SRC)/util.c               \
        $(SRC)/video.c

# -----------------------------------------------------------------------------
# *.o bject files for linkage stage of the C-kernel ...
# -----------------------------------------------------------------------------
OBJS := $(OBJ)/ckernel.o            \
        $(OBJ)/descriptor_table.o   \
        $(OBJ)/flpydisk.o           \
        $(OBJ)/fs.o                 \
        $(OBJ)/gdt.o                \
        $(OBJ)/idt.o                \
        $(OBJ)/initrd.o             \
        $(OBJ)/irq.o                \
        $(OBJ)/isrs.o               \
        $(OBJ)/keyboard.o           \
        $(OBJ)/kheap.o              \
        $(OBJ)/math.o               \
        $(OBJ)/ordered_array.o      \
        $(OBJ)/paging.o             \
        $(OBJ)/shared_pages.o       \
        $(OBJ)/syscall.o            \
        $(OBJ)/task.o               \
        $(OBJ)/timer.o              \
        $(OBJ)/util.o               \
        $(OBJ)/video.o

all: config\
     $(BIN)/INITRD.EXE $(BIN)/BOOT1.SYS $(BIN)/BOOT2.SYS $(BIN)/KERNEL.SYS

# -----------------------------------------------------------------------------
# loader to load the system from DOS console ...
# -----------------------------------------------------------------------------
$(BIN)/BOOT1.SYS: $(SRC)/boot1.asm
	$(AS) -f bin $< -o $@

$(BIN)/BOOT2.SYS: $(SRC)/boot2.asm
	$(AS) -f bin $(SRC)/boot2.asm -o $(BIN)/BOOT2.SYS

# -----------------------------------------------------------------------------
# compile all *.c files to *.o bject files ...
# -----------------------------------------------------------------------------
$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# -----------------------------------------------------------------------------
# compile all *.asm files to *.o bject files ...
# -----------------------------------------------------------------------------
$(OBJ)/%.o: $(SRC)/%.asm
	$(AS) $(ASMFLAGS) $< -o $@

$(OBJ)/kernel.coff: $(OBJS) $(SRC)/kernel.ld
	$(LD) $(LDFLAGS) -m i386pe -o $@ $(OBJS)

# -----------------------------------------------------------------------------
# link C-kernel to finaly output binary image ...
# -----------------------------------------------------------------------------
$(BIN)/KERNEL.SYS: $(OBJS)
	$(LD) $(LDFLAGS) $+ -o $@

# -----------------------------------------------------------------------------
$(OBJ)/data.o: $(OBJ)/initrd.dat $(SRC)/data.asm
	$(AS) $(ASMFLAGS) $(SRC)/data.asm -o $(OBJ)/data.o
    
# -----------------------------------------------------------------------------
# create the initial ram disk ...
# -----------------------------------------------------------------------------
$(OBJ)/initrd.dat: $(OBJ)/make_initrd.o
	$(BIN)/INITRD.EXE                 \
    $(SRC)/content/test1.txt   file1  \
    $(SRC)/content/test2.txt   file2  \
    $(SRC)/content/test3.txt   file3  \
    $(SRC)/content/program.elf shell

$(BIN)/INITRD.EXE: $(OBJ)/initrd.dat
	$(CC) -m32 -mconsole -O2 -Wall -Wextra -o $@ $<
	$(STRIP) $@

# -----------------------------------------------------------------------------
# create directories for output object code ...
# -----------------------------------------------------------------------------
config:
	$(MKDIR) -p $(BIN)
	$(MKDIR) -p $(OBJ)

# -----------------------------------------------------------------------------
# before you compile, you can clean the source tree from old crap ...
# -----------------------------------------------------------------------------
clean:
	$(RM) -f  $(OBJ)/*.o $(OBJ)/*.coff
	$(RM) -f  $(BIN)/*.BIN $(BIN)/*.COM $(BIN)/*.SYS
	$(RM) -rf $(OBJ)
