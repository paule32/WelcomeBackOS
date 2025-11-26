CROSS    ?= 
AS       := nasm
RM       := rm
MKDIR    := mkdir
CC       := $(CROSS)gcc
LD       := $(CROSS)ld
OBJCOPY  := $(CROSS)objcopy

CFLAGS   := -m32 -O2 -ffreestanding -fno-builtin -nostdlib -nostartfiles \
            -Wall -Wextra -Wno-unused-value -Wno-unused-parameter

LDFLAGS  := -nostdlib -T ../src/kernel.ld
ASMFLAGS := -f win32 -O2 -DDOS_MODE=32

SRCC := ../src/kernel.c ../src/util.c ../src/math.c
OBJS := ../obj/kernel.o ../obj/util.o ../obj/math.o ../obj/isr.o

all: dirs ../bin/LOADER.COM ../bin/KERNEL.BIN

../bin/LOADER.COM: ../src/loader.asm
	$(AS) -f bin $< -o $@

../obj/%.o: ../src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

../obj/%.o: ../src/%.asm
	$(AS) $(ASMFLAGS) $< -o $@

../obj/kernel.coff: $(OBJS) ../src/kernel.ld
	$(LD) $(LDFLAGS) -m i386pe -o $@ $(OBJS)

../bin/KERNEL.BIN: ../obj/kernel.coff
	$(OBJCOPY) -O binary $< $@

dirs:
	$(MKDIR) -p ../bin
	$(MKDIR) -p ../obj

clean:
	$(RM) -f *.o *.coff *.BIN *.COM
	$(RM) -rf ../bin ../obj
