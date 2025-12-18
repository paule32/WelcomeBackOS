# include "stdint.h"
# include "proto.h"
# include "vga.h"

static vbe_info_t* _vga;

uint32_t lfb_base ;
uint16_t lfb_pitch;
uint16_t lfb_xres ;
uint16_t lfb_yres ;
uint8_t  lfb_bpp  ;
uint32_t lfb_phys ;

void gfx_init(void)
{
    _vga = (vbe_info_t*)0x00009000;
    
    lfb_base  = _vga->phys_base;
    lfb_pitch = _vga->pitch; 
    lfb_bpp   = _vga->bpp ;
    
    lfb_xres  = _vga->xres;
    lfb_yres  = _vga->yres;
    
    printformat("--> X: %d, Y: %d, BPP: %d, ADDR: 0x%x\n",
    lfb_xres,lfb_yres,lfb_bpp,lfb_base);
    
    // z.B.: nur 16bpp-Modus erstmal unterstützen
    if (lfb_bpp != 16) {
        // Fallback
        return;
    }
}

void gfx_clear(
    USHORT color) {
    for (int y = 0; y < lfb_yres; ++y) {
        UINT base_off = (UINT)y * (UINT)lfb_pitch;
        volatile USHORT* p = (volatile USHORT*)(lfb_base + base_off);
        for (int x = 0; x < lfb_xres; ++x) {
            *p++ = color;
        }
    }
}

USHORT rgb565(
    UCHAR r,
    UCHAR g,
    UCHAR b) {
    
    USHORT R = (r >> 3) & 0x1F;  // 5 Bit
    USHORT G = (g >> 2) & 0x3F;  // 6 Bit
    USHORT B = (b >> 3) & 0x1F;  // 5 Bit
    
    return (USHORT)((R << 11) | (G << 5) | B);
}

void gfx_putPixel(
    int x,
    int y,
    USHORT color) {
        
    if (x < 0 || y < 0 || x >= lfb_xres || y >= lfb_yres)
    return;

    volatile USHORT* row = (volatile USHORT*)(lfb_base + (UINT)y * (UINT)lfb_pitch);
    row[x] = color;
}

void gfx_rectFill(
    int x,
    int y,
    int w,
    int h,
    USHORT color) {
        
    // --- Clipping ---
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }

    if (x + w > lfb_xres) w = lfb_xres - x;
    if (y + h > lfb_yres) h = lfb_yres - y;

    if (w <= 0 || h <= 0)
        return;

    // --- Füllen ---
    for (int yy = y; yy < y + h; ++yy) {
        // Startadresse der Zeile yy, Pixel x
        UINT base_off = (UINT)yy * (UINT)lfb_pitch + (UINT)x * 2; // 2 Bytes pro Pixel (16-bit)
        volatile USHORT* p = (volatile USHORT*)(lfb_base + base_off);

        for (int xx = 0; xx < w; ++xx) {
            *p++ = color;
        }
    }
}
