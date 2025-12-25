CROSS ?= i686-elf-
CC    := $(CROSS)gcc
AR    := $(CROSS)ar
AS    := $(CROSS)gcc
LD    := $(CROSS)ld

CFLAGS := -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -nostartfiles -O2
ASFLAGS:= -m32 -ffreestanding
LDFLAGS:= -m elf_i386 -T app.ld

all: app.elf

# User-API Library
kapi.o: kapi.c kapi.h syscall_stub.h
	$(CC) $(CFLAGS) -c kapi.c -o kapi.o

libkapi.a: kapi.o
	$(AR) rcs $@ $^

# CRT0
crt0.o: crt0.S
	$(AS) $(ASFLAGS) -c crt0.S -o crt0.o

# App
app.o: app.c kapi.h
	$(CC) $(CFLAGS) -c app.c -o app.o

app.elf: crt0.o app.o libkapi.a app.ld
	$(LD) $(LDFLAGS) -o $@ crt0.o app.o -L. -lkapi
