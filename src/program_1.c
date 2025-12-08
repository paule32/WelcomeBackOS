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
#include "vbe.h"

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

ULONG   lfb_phys  = 0;

static UINT* backbuffer        = 0;  // 16-bpp-Backbuffer
static UINT  back_pitch_pixels = 0;  // in Pixeln

typedef struct {
    int x;
    int y;
} FillPoint;

// 800*600 = 480000 → etwas Reserve
#define FILL_STACK_MAX 500000

static FillPoint fill_stack[FILL_STACK_MAX];
static int       fill_sp = 0;

void vbe_read_modeinfo_early(void)
{
    printformat("VBE: 0x%x\n", lfb_base);
    printformat("X=%d Y=%d BPP=%d\n",
                lfb_xres,
                lfb_yres,
                lfb_bpp
    );
}

void vbe_init_pm(void)
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

void gfx_init_backbuffer(void)
{
    // 16bpp -> 2 Bytes pro Pixel
    back_pitch_pixels = lfb_pitch / 2;

    // Backbuffer in normalem RAM – hier einfache, statische Variante:
    // Achtung: 800*600*2 = 960000 Bytes ~ 0,9 MB
    backbuffer = (UINT*)k_malloc(
        back_pitch_pixels * lfb_yres * sizeof(UINT),
        0, 0
    );

    if (!backbuffer) {
        // notfalls direkt im LFB weiterzeichnen
        // oder Fehlermeldung
        return;
    }

    // Bildschirm & Backbuffer löschen
    UINT size = (UINT)back_pitch_pixels * lfb_yres;
    for (UINT i = 0; i < size; ++i)
    backbuffer[i] = 0;
}


void ff_push(int x, int y)
{
    if (fill_sp >= FILL_STACK_MAX) return;
    fill_stack[fill_sp].x = x;
    fill_stack[fill_sp].y = y;
    fill_sp++;
}

int ff_pop(int* x, int* y)
{
    if (fill_sp <= 0) return 0;
    fill_sp--;
    *x = fill_stack[fill_sp].x;
    *y = fill_stack[fill_sp].y;
    return 1;
}

void gfx_present(void)
{
    // Zeile für Zeile kopieren (falls pitch != xres*2)
    for (USHORT y = 0; y < lfb_yres; ++y) {
        // Quelle im Backbuffer (in Pixeln)
        UINT* src = &backbuffer[(UINT)y * back_pitch_pixels];

        // Ziel im LFB (in Bytes)
        UINT dst_off = (UINT)y * lfb_pitch;
        volatile UINT* dst = (volatile UINT*)(lfb_base + dst_off);

        for (UINT x = 0; x < lfb_xres; ++x) {
            dst[x] = src[x];
        }
    }
}

USHORT get_pixel_back(
    int x,
    int y) {
        
    if (x < 0 || y < 0 || x >= (int)lfb_xres || y >= (int)lfb_yres)
    return 0;
    
    UINT index = (UINT)y * back_pitch_pixels + (UINT)x;
    return backbuffer[index];
}

void put_pixel(
    int x,
    int y,
    USHORT color) {
    
    if (x < 0 || y < 0 || x >= (int)lfb_xres || y >= (int)lfb_yres)
    return;

    UINT index = (UINT)y * back_pitch_pixels + (UINT)x;
    backbuffer[index] = color;
}

void put_pixel_back(
    int x,
    int y,
    USHORT color) {
        
    if (x < 0 || y < 0 || x >= (int)lfb_xres || y >= (int)lfb_yres)
        return;

    UINT index = (UINT)y * back_pitch_pixels + (UINT)x;
    backbuffer[index] = color;
}

void put_thick_pixel(
    int x,
    int y,
    int thickness,
    USHORT color) {
        
    if (thickness <= 1) {
        put_pixel(x, y, color);
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

void put_thick_pixel_back(
    int x,
    int y,
    int thickness,
    USHORT color) {
        
    if (thickness <= 1) {
        put_pixel_back(x, y, color);
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
        UINT row_index = (UINT)yy * back_pitch_pixels + (UINT)x0;
        UINT* p = &backbuffer[row_index];
        for (int xx = x0; xx <= x1; ++xx) {
            *p++ = color;
        }
    }
}

void draw_line_thick(
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
            put_thick_pixel_back(x0, y0, thickness, color);

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
            put_thick_pixel_back(x0, y0, thickness, color);

            y0 += sy;
            err -= dx;
            if (err < 0) {
                x0 += sx;
                err += dy;
            }
        }
    }
}

void draw_circle(
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
        put_pixel_back(cx + x, cy + y, color);
        put_pixel_back(cx + y, cy + x, color);
        put_pixel_back(cx - y, cy + x, color);
        put_pixel_back(cx - x, cy + y, color);  // <--- not commented => crash
        put_pixel_back(cx - x, cy - y, color);
        put_pixel_back(cx - y, cy - x, color);
        put_pixel_back(cx + y, cy - x, color);
        put_pixel_back(cx + x, cy - y, color);

        y++;

        if (err < 0) {
            err += 2*y + 1;
        } else {
            x--;
            err += 2*(y - x) + 1;
        }
    }
}

void draw_circle_thick(
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
        draw_circle(cx, cy, r, color);
    }
}

void gfx_clear(USHORT color)
{
    UINT size = (UINT)back_pitch_pixels * lfb_yres;
    
    for (UINT i = 0; i < size; i++)
    backbuffer[i] = color;
}

void gfx_rect_fill(
    int x,
    int y,
    int w,
    int h,
    USHORT color) {
        
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    
    if (x + w > lfb_xres) w = lfb_xres-x;
    if (y + h > lfb_yres) h = lfb_yres-y;
    
    if (w <= 0 || h <= 0)
    return;

    for (int yy = y; yy < y + h; yy++) {
        UINT  pos = (UINT)yy * back_pitch_pixels + x;
        UINT* row = &backbuffer[pos];
        for (int xx = 0; xx < w; xx++)
        row[xx] = color;
    }
}

void gfx_rect_frame(
    int x,
    int y,
    int w, 
    int h,
    int thick,
    USHORT color) {
        
    gfx_rect_fill(x, y, w, thick, color);                 // oben
    gfx_rect_fill(x, y+h-thick, w, thick, color);         // unten
    gfx_rect_fill(x, y, thick, h, color);                 // links
    gfx_rect_fill(x+w-thick, y, thick, h, color);         // rechts
}

//extern void init_vbe (void);
//extern void start_gui(void);

void user_program_1(void)
{
    /*
    k_clear_screen();
    settextcolor(14, 1);
    set_cursor(0, 0);
    for (int i = 0; i < 79 * 25 + 24; ++i) {
        putch('O');
    }
    set_cursor(0, 0);*/
    
    gfx_init_backbuffer();
    
    USHORT red, green, blue;
    
    red   = rgb565(255, 0, 0);
    green = rgb565(0, 255, 0);
    blue  = rgb565(0, 0, 255);

    // drei Pixel zeichnen
    put_pixel_back(100, 50, red  );
    put_pixel_back(101, 50, green);
    put_pixel_back(102, 50, blue );

    for (int y = 1; y < 50; y++) {
        for (int x = 1; x < 20; x++) {
            put_pixel_back(120+x, 150+y, green);
        }
    }

#if 0
    // dünne Linie
    draw_line_thick(50,  50, 300, 100, red, 1);
    // mitteldick
    draw_line_thick(50, 150, 300, 250, grn, 3);
    // sehr dick
    draw_line_thick(50, 300, 300, 450, blu, 7);
#endif
    
    
    // Backbuffer löschen
    for (UINT i = 0; i < (UINT)back_pitch_pixels * lfb_yres; ++i)
        backbuffer[i] = 0;

    red   = rgb565(255, 0,   0);
    green = rgb565(0,   255, 0);
    blue  = rgb565(0,   0,   255);

    draw_line_thick(50,  50, 300, 100, red,   1);  // dünn
    draw_line_thick(50, 150, 300, 250, green, 4);  // dicker
    draw_line_thick(50, 300, 300, 450, blue,  8);  // sehr dick

    gfx_rect_fill(300, 300, 100, 42, green);
    
    gfx_rect_fill (50, 50, 300, 200,    rgb565(0  , 120, 255));  // Block
    gfx_rect_frame(50, 50, 300, 200, 4, rgb565(255, 255, 255));  // Rahmen
    
    
    // dünn
    draw_circle_thick(200, 150, 50, 1, red);
    // mittel
    draw_circle_thick(400, 300, 80, 4, green);
    // sehr dick
    draw_circle_thick(600, 400, 60, 8, blue);
    
    // alles auf einmal anzeigen
    gfx_present();
    
    for(;;);
    //init_vbe();
    //start_gui();
}
