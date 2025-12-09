// ---------------------------------------------------------------------------
// \file  program_1.c – A simple freestanding C-Kernel
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
# include "os.h"
# include "kheap.h"
# include "task.h"
# include "initrd.h"
# include "syscall.h"
# include "shared_pages.h"
# include "flpydsk.h"
# include "vbe.h"
# include "math.h"

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

#define VBE_MODE_INFO_PTR ((const VbeModeInfo*)0x00090000)

/* globale Variablen für dein Video-System */
volatile UCHAR*  lfb_base  = 0;
USHORT  lfb_pitch = 0;
USHORT  lfb_xres  = 0;
USHORT  lfb_yres  = 0;
UCHAR   lfb_bpp   = 0;

void vbe_read_modeinfo_early(void)
{
    printformat((char*)"VBE: 0x%x\n", lfb_base);
    printformat((char*)"X=%d Y=%d BPP=%d\n",
                lfb_xres,
                lfb_yres,
                lfb_bpp
    );
}

extern "C" void vbe_init_pm(void)
{
    const VbeModeInfo* mi = VBE_MODE_INFO_PTR;

    lfb_base  = (volatile UCHAR*)(UINT*)mi->PhysBasePtr;
    lfb_pitch = mi->BytesPerScanLine;
    lfb_xres  = mi->XResolution;
    lfb_yres  = mi->YResolution;
    lfb_bpp   = mi->BitsPerPixel;
    
    vbe_read_modeinfo_early();
}

USHORT rgb565(UCHAR r, UCHAR g, UCHAR b)
{
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
    USHORT color,
    int thickness) {
        
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

        int dx = 1; //(int)(k_sqrt((double)(r2 - dy2)) + 0.5); // leicht gerundet

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

void gfx_clear(USHORT color) {
    gfx_rectFill(0,0,lfb_xres,lfb_yres,color);
}

void call_002(void)
{
    gfx_rectFill (350, 350, 200, 100,    rgb565(0  , 120, 255));  // Block
    gfx_rectFill (30, 350, 20, 100,    rgb565(40  , 120, 255));  // Block
    gfx_drawLine(50,  50, 300, 100, rgb565(255, 0, 0), 1);
}
extern "C" void user_program_1(void)
{
    /*
    k_clear_screen();
    settextcolor(14, 1);
    set_cursor(0, 0);
    for (int i = 0; i < 79 * 25 + 24; ++i) {
        putch('O');
    }
    set_cursor(0, 0);*/
    
    USHORT red, green, blue;
    
    red   = rgb565(255, 0, 0);
    green = rgb565(0, 255, 0);
    blue  = rgb565(0, 0, 255);

    // drei Pixel zeichnen
    gfx_putPixel(100, 50, red  );
    gfx_putPixel(101, 50, green);
    gfx_putPixel(102, 50, blue );

    for (int y = 1; y < 50; y++) {
        for (int x = 1; x < 20; x++) {
            gfx_putPixel(120+x, 150+y, green);
        }
    }

#if 0
    // dünne Linie
    gfx_drawLine(50,  50, 300, 100, red, 1);
    // mitteldick
    gfx_drawLine(50, 150, 300, 250, grn, 3);
    // sehr dick
    gfx_drawLine(50, 300, 300, 450, blu, 7);
#endif
    
    red   = rgb565(255, 0,   0);
    green = rgb565(0,   255, 0);
    blue  = rgb565(0,   0,   255);

    gfx_drawLine(50,  50, 300, 100, red,   1);  // dünn
    gfx_drawLine(50, 150, 300, 250, green, 4);  // dicker
    gfx_drawLine(50, 300, 300, 450, blue,  8);  // sehr dick

    gfx_rectFill(300, 300, 100, 42, green);
    
    gfx_rectFill (50, 50, 300, 200,    rgb565(0  , 120, 255));  // Block
    gfx_rectFrame(50, 50, 300, 200, 4, rgb565(255, 255, 255));  // Rahmen
    
    
    // dünn
    gfx_drawCircle(200, 150, 50, 1, red);
    // mittel
    gfx_drawCircle(400, 300, 80, 4, green);
    // sehr dick
    gfx_drawCircle(600, 400, 60, 8, blue);
    
    gfx_drawCircle(200, 200, 60, 8, blue);
    //gfx_drawCircleFill(260, 200, 50, red);
    
    gfx_rectFill (50, 50, 300, 100,    rgb565(0  , 120, 255));  // Block
    
    call_002();
    
    gfx_rectFill (30, 350, 20, 100,    rgb565(40  , 120, 255));  // Block
    gfx_rectFill (50, 350, 20, 100,    rgb565(40  , 120, 255));  // Block
    
    
    gfx_putPixel(100, 50, red  );
    gfx_putPixel(101, 50, green);
    gfx_putPixel(102, 50, blue );
    
    for(;;);
    //init_vbe();
    //start_gui();
}
