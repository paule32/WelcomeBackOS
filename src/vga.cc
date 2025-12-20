# include "stdint.h"
# include "proto.h"
#define DESKTOP
# include "vga.h"
# include "math.h"

# define VBE_MODE_INFO_PTR ((const vbe_info_t*)0x00009000)

static vbe_info_t* _vga = (vbe_info_t*)0x00009000;

volatile uint32_t lfb_base  = 0;

USHORT  lfb_pitch = 0;
USHORT  lfb_xres  = 0;
USHORT  lfb_yres  = 0;
UCHAR   lfb_bpp   = 0;

int gfx_init(void)
{
    const vbe_info_t* mi = VBE_MODE_INFO_PTR;

    lfb_base  = mi->phys_base;
    lfb_pitch = mi->pitch; 
    lfb_bpp   = mi->bpp ;
    
    lfb_xres  = mi->xres;
    lfb_yres  = mi->yres;
    
    printformat("--> X: %d, Y: %d, BPP: %d, ADDR: 0x%x\n",
    lfb_xres,lfb_yres,lfb_bpp,lfb_base);
    
    // z.B.: nur 16bpp-Modus erstmal unterstützen
    if (lfb_bpp != 16) {
        // Fallback
        return -1;
    }
    
    size_t fb_bytes = (size_t)lfb_pitch * (size_t)lfb_yres;

    // WICHTIG: phys -> virt mappen
    uintptr_t lfb_virt = (uintptr_t)mmio_map(lfb_base, (uint32_t)fb_bytes);

    // Ab jetzt IMMER die virtuelle Adresse zum Schreiben benutzen!
    lfb_base = (uint32_t)lfb_virt;

    return  0;
}

void gfx_clear(uint16_t color)
{
    for (uint16_t y = 0; y < lfb_yres; ++y) {
        uint32_t off = y * lfb_pitch;
        volatile uint16_t* p = (volatile uint16_t*)(lfb_base + off);
        for (uint16_t x = 0; x < lfb_xres; ++x) *p++ = color;
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

void gfx_hLine(
    int x0,
    int x1,
    int y,
    USHORT color) {
        
    if (y < 0 || y >= lfb_yres)
    return;

    if (x0 > x1) {
        int t = x0; x0 = x1; x1 = t;
    }

    if (x1 < 0 || x0 >= lfb_xres)
    return;

    if (x0 < 0)         x0 = 0;
    if (x1 >= lfb_xres) x1 = lfb_xres - 1;

    volatile USHORT* row = (volatile USHORT*)(lfb_base + (UINT)y * (UINT)lfb_pitch);

    for (int x = x0; x <= x1; ++x)
    row[x] = color;
}

USHORT gfx_getPixel(
    int x,
    int y)
{
    // Optional: Bounds-Check
    if (x < 0 || y < 0 || x >= lfb_xres || y >= lfb_yres)
        return 0;   // oder irgendein Fehlerwert

    // Zeilenanfang in Bytes: y * pitch
    // Dann x * 2 (weil 2 Bytes pro Pixel)
    UINT base_off = (UINT)y * (UINT)lfb_pitch + (UINT)x * 2;

    volatile USHORT* p = (volatile USHORT*)(lfb_base + base_off);
    return *p;
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

void gfx_putPixel(
    int x,
    int y,
    int thickness,
    USHORT color) {
        
    if (thickness <= 1) {
        gfx_putPixel(x, y, color);
        return;
    }

    int half = thickness / 2;

    int y0 = y - half;
    int y1 = y + half;
    int x0 = x - half;
    int x1 = x + half;

    if (y0 < 0) y0 = 0;
    if (x0 < 0) x0 = 0;
    if (y1 >= (int)lfb_yres) y1 = lfb_yres - 1;
    if (x1 >= (int)lfb_xres) x1 = lfb_xres - 1;

    for (int yy = y0; yy <= y1; ++yy) {
        UINT base_off = (UINT)yy * lfb_pitch + (UINT)x0 * 2;
        volatile USHORT* p = (volatile USHORT*)(lfb_base + base_off);
        for (int xx = x0; xx <= x1; ++xx) {
            *p++ = color;
        }
    }
}

void gfx_drawLine(
    int x0, int y0,
    int x1, int y1,
    int thickness ,
    USHORT color) {
        
    int dx = x1 - x0;
    int dy = y1 - y0;

    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;

    dx = sx * dx;
    dy = sy * dy;

    int err, e2;

    if (dx >= dy) {
        // flache Linie
        err = dx / 2;
        for (int i = 0; i <= dx; ++i) {
            gfx_putPixel(x0, y0, thickness, color);

            x0 += sx;
            err -= dy;
            if (err < 0) {
                y0 += sy;
                err += dx;
            }
        }
    } else {
        // steile Linie
        err = dy / 2;
        for (int i = 0; i <= dy; ++i) {
            gfx_putPixel(x0, y0, thickness, color);

            y0 += sy;
            err -= dx;
            if (err < 0) {
                x0 += sx;
                err += dy;
            }
        }
    }
}

void gfx_drawCircle(
    int cx,
    int cy,
    int radius,
    USHORT color) {
        
    if (radius <= 0) return;

    int x = radius;
    int y = 0;
    int err = 1 - radius;

    while (x >= y)
    {
        // 8 symmetrische Punkte
        gfx_putPixel(cx + x, cy + y, color);
        gfx_putPixel(cx + y, cy + x, color);
        gfx_putPixel(cx - y, cy + x, color);
        gfx_putPixel(cx - x, cy + y, color);  // <--- not commented => crash
        gfx_putPixel(cx - x, cy - y, color);
        gfx_putPixel(cx - y, cy - x, color);
        gfx_putPixel(cx + y, cy - x, color);
        gfx_putPixel(cx + x, cy - y, color);

        y++;

        if (err < 0) {
            err += 2*y + 1;
        } else {
            x--;
            err += 2*(y - x) + 1;
        }
    }
}

void gfx_drawCircle(
    int cx,
    int cy,
    int radius,
    int thickness,
    USHORT color) {
        
    //if (radius <= 0 || thickness <= 0)
    //return;

    int half = thickness / 2;
    int r_start = radius - half;
    int r_end   = radius + (thickness - half - 1);

    if (r_start < 1)
        r_start = 1;
    
    for (int r = 1; r <= r_end; ++r)
    {
        gfx_drawCircle(cx, cy, r, color);
    }
}

void gfx_drawCircleFill(
    int cx,
    int cy,
    int r,
    USHORT color) {

    if (r <= 0)
    return;

    int r2 = r * r;

    for (int dy = -r; dy <= r; ++dy) {
        int y = cy + dy;
        if (y < 0 || y >= lfb_yres)
        continue;

        int dy2 = dy * dy;
        if (dy2 > r2)
        continue;

        int dx = (int)(k_sqrt((double)(r2 - dy2)) + 0.5); // leicht gerundet

        int x0 = cx - dx;
        int x1 = cx + dx;

        for (int x = x0; x <= x1; ++x) {
            gfx_putPixel(x, y, color);
        }
    }
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

void gfx_rectFrame(
    int x,
    int y,
    int w, 
    int h,
    int thick,
    USHORT color) {
        
    gfx_rectFill(x, y, w, thick, color);                 // oben
    gfx_rectFill(x, y+h-thick, w, thick, color);         // unten
    gfx_rectFill(x, y, thick, h, color);                 // links
    gfx_rectFill(x+w-thick, y, thick, h, color);         // rechts
}
