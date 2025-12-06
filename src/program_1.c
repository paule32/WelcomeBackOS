// ---------------------------------------------------------------------------
// \file  program_1.c – A simple freestanding C-Kernel
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
#include "os.h"
#include "kheap.h"
#include "task.h"
#include "initrd.h"
#include "syscall.h"
#include "shared_pages.h"
#include "flpydsk.h"

typedef struct {
    USHORT ModeAttributes;
    UCHAR  WinAAttributes;
    UCHAR  WinBAttributes;
    USHORT WinGranularity;
    USHORT WinSize;
    USHORT WinASegment;
    USHORT WinBSegment;
    UINT   WinFuncPtr;
    USHORT BytesPerScanLine;

    USHORT XResolution;
    USHORT YResolution;
    UCHAR  XCharSize;
    UCHAR  YCharSize;
    UCHAR  NumberOfPlanes;
    UCHAR  BitsPerPixel;
    UCHAR  NumberOfBanks;
    UCHAR  MemoryModel;
    UCHAR  BankSize;
    UCHAR  NumberOfImagePages;
    UCHAR  Reserved1;

    UCHAR  RedMaskSize;
    UCHAR  RedFieldPosition;
    UCHAR  GreenMaskSize;
    UCHAR  GreenFieldPosition;
    UCHAR  BlueMaskSize;
    UCHAR  BlueFieldPosition;
    UCHAR  RsvdMaskSize;
    UCHAR  RsvdFieldPosition;
    UCHAR  DirectColorModeInfo;

    UINT PhysBasePtr;      // <- das Feld, das du suchst
    UINT OffScreenMemOffset;
    USHORT OffScreenMemSize;
    // Rest ignoriert
} __attribute__((packed)) VbeModeInfo;

#define VBE_MODE_INFO_PTR ((VbeModeInfo*)0x00009000)

/* globale Variablen für dein Video-System */
UCHAR*  lfb_base  = 0;
USHORT  lfb_pitch = 0;
USHORT  lfb_xres  = 0;
USHORT  lfb_yres  = 0;
UCHAR   lfb_bpp   = 0;

void vbe_init_pm(void)
{
    const VbeModeInfo* mi = VBE_MODE_INFO_PTR;

    lfb_base  = (UCHAR*)(UINT*)mi->PhysBasePtr;
    lfb_pitch = mi->BytesPerScanLine;
    lfb_xres  = mi->XResolution;
    lfb_yres  = mi->YResolution;
    lfb_bpp   = mi->BitsPerPixel;
}

USHORT rgb565(UCHAR r, UCHAR g, UCHAR b)
{
    USHORT R = (r >> 3) & 0x1F;  // 5 Bit
    USHORT G = (g >> 2) & 0x3F;  // 6 Bit
    USHORT B = (b >> 3) & 0x1F;  // 5 Bit
    
    return (USHORT)((R << 11) | (G << 5) | B);
}

void put_pixel(
    USHORT x,
    USHORT y,
    USHORT color) {
    if (x >= lfb_xres || y >= lfb_yres)
        return;
    // offset in Bytes: y * pitch + x * 2
    UINT offset = (UINT)y * lfb_pitch + (UINT)x * 2;

    USHORT* p = (USHORT*)(lfb_base + offset);
    *p = color;
}

//extern void init_vbe (void);
//extern void start_gui(void);

void user_program_1(void)
{
    k_clear_screen();
    settextcolor(14, 1);
    set_cursor(0, 0);
    for (int i = 0; i < 79 * 25 + 24; ++i) {
        putch('O');
    }
    set_cursor(0, 0);
    
    vbe_init_pm();
    
    USHORT red = rgb565(255, 0, 0);
    USHORT grn = rgb565(0, 255, 0);
    USHORT blu = rgb565(0, 0, 255);

    // drei Pixel zeichnen
    put_pixel(100, 50, red);
    put_pixel(101, 50, grn);
    put_pixel(102, 50, blu);
    
    //init_vbe();
    //start_gui();
}
