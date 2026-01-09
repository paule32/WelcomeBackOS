// ----------------------------------------------------------------------------
// \file  clear_screen.c
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# define VGA_MEM ((volatile unsigned short*)0xB8000)
# define COLS 80
# define ROWS 25

static inline void outb(unsigned short port, unsigned char val)
{
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

void clear_screen(void)
{
    for (int i = 0; i < COLS * ROWS; i++)
    VGA_MEM[i] = 0x0700 | ' ';

    // Cursor auf 0:0
    outb(0x3D4, 0x0F);
    outb(0x3D5, 0);
    outb(0x3D4, 0x0E);
    outb(0x3D5, 0);
}
