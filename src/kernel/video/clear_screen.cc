// ----------------------------------------------------------------------------
// \file  clear_screen.c
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"

# define VGA_MEM ((volatile unsigned short*)0xB8000)
# define COLS 80
# define ROWS 25

static inline void outb(unsigned short port, unsigned char val)
{
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

void clear_screen(unsigned char fg, unsigned char bg)
{
    uint8_t  attr = (uint8_t)(((bg & 0x0F) << 4) | (fg & 0x0F));
    uint16_t cell = (uint16_t)(bg | ((uint16_t)fg << 8));

    for (int i = 0; i < COLS * ROWS; i++)
    VGA_MEM[i] = cell;

    // Cursor auf 0:0
    outb(0x3D4, 0x0F);
    outb(0x3D5, 0);
    outb(0x3D4, 0x0E);
    outb(0x3D5, 0);
}
