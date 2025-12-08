# -----------------------------------------------------------------------------
# \file  code32.asm
# \note  (c) 2025 by Jens Kallup - paule32
#        all rights reserved.
# -----------------------------------------------------------------------------
CROSS    ?= 

EXPORT   := export
AS       := nasm
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
CFLAGS   := -m32 -O0 -ffreestanding -fno-stack-protector -fno-builtin   \
            -nostdlib -nostartfiles -Wall -Wextra                       \
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
        $(SRC)/video.c              \
        $(SRC)/program_1.cc

# -----------------------------------------------------------------------------
# *.o bject files for linkage stage of the C-kernel ...
# -----------------------------------------------------------------------------
OBJS := $(OBJ)/ckernel.o            \
        $(OBJ)/data.o               \
        $(OBJ)/descriptor_table.o   \
        $(OBJ)/flpydisk.o           \
        $(OBJ)/flush.o              \
        $(OBJ)/fs.o                 \
        $(OBJ)/gdt.o                \
        $(OBJ)/idt.o                \
        $(OBJ)/initrd.o             \
        $(OBJ)/irq.o                \
        $(OBJ)/isr.o                \
        $(OBJ)/isrs.o               \
        $(OBJ)/keyboard.o           \
        $(OBJ)/kheap.o              \
        $(OBJ)/math.o               \
        $(OBJ)/ordered_array.o      \
        $(OBJ)/paging.o             \
        $(OBJ)/process.o            \
        $(OBJ)/shared_pages.o       \
        $(OBJ)/syscall.o            \
        $(OBJ)/task.o               \
        $(OBJ)/timer.o              \
        $(OBJ)/util.o               \
        $(OBJ)/video.o              \
        $(OBJ)/video_fb.o           \
        $(OBJ)/program_1.o

all: config\
     $(OBJ)/data.o $(BIN)/BOOT2.SYS $(BIN)/floppy.img

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
	$(LD) $(LDFLAGS) -o $(OBJ)/kernel.o $(OBJS)
#$(CC) -o $(OBJ)/kernel.o $(OBJS)

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

$(BIN)/KERNEL.SYS: $(OBJ)/kernel.o
	$(OBJCOPY) -O binary $(OBJ)/kernel.o $(SRC)/content/KERNEL.SYS

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
	    } else { \
            print "Error: Bootsignatur incorrect. (" $$1 " " $$2 ")"; \
            exit 1;  \
	    } }'
	cp $(BIN)/BOOT2.SYS $(SRC)/content/BOOT2.SYS
	mcopy -i $(BIN)/floppy.img $(SRC)/content/* ::/

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
	$(RM) -f  $(BIN)/*.BIN $(BIN)/*.COM $(BIN)/*.SYS $(BIN)/kernel.map
	$(RM) -f  $(SRC)/content/*.SYS
	$(RM) -f  $(SRC)/initrd.dat
	$(RM) -f  $(BIN)/floppy.img
	$(RM) -rf $(OBJ)
