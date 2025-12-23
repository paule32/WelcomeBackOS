# include "stdint.h"
# include "kheap.h"
# include "iso9660.h"

# define DESKTOP
# include "vga.h"
# include "bitmap.h"

// ---- BMP constants ----
#define BI_RGB       0
#define BI_BITFIELDS 3

extern uint32_t file_read (FILE* f, void* buf, uint32_t len);
extern int      file_seek (FILE* f, uint32_t new_pos);
extern void     file_close(FILE* f);

#define BI_RGB       0
#define BI_BITFIELDS 3

typedef struct __attribute__((packed)) {
    uint16_t bfType;      // 'BM'
    uint32_t bfSize;
    uint16_t bfRes1;
    uint16_t bfRes2;
    uint32_t bfOffBits;
} BMP_FILEHDR;

typedef struct __attribute__((packed)) {
    uint32_t biSize;        // 40
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;      // 1
    uint16_t biBitCount;    // 16
    uint32_t biCompression; // 0 or 3
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMP_INFOHDR;

static bool read_exact(FILE* f, void* buf, uint32_t len) {
    uint32_t got = file_read(f, buf, len);
    return got == len;
}

extern "C" bool bmp_show_from_iso_16bpp565(
    const char* path,
    uint8_t* lfb,
    uint32_t pitch,
    int screen_w, int screen_h,
    int dst_x, int dst_y)
{
    FILE* f = file_open(path);
    if (!f) { gfx_printf("bmp: open failed\n"); return false; }

    BMP_FILEHDR fh;
    BMP_INFOHDR ih;

    if (!read_exact(f, &fh, sizeof(fh))) { gfx_printf("bmp: read fh failed\n"); file_close(f); return false; }
    if (fh.bfType != 0x4D42)             { gfx_printf("bmp: bfType=%x\n", fh.bfType); file_close(f); return false; }

    if (!read_exact(f, &ih, sizeof(ih))) { gfx_printf("bmp: read ih failed\n"); file_close(f); return false; }

    gfx_printf("bmp: off=%u w=%d h=%d bpp=%u comp=%u\n",
               fh.bfOffBits, (int)ih.biWidth, (int)ih.biHeight,
               (unsigned)ih.biBitCount, (unsigned)ih.biCompression);

    if (ih.biSize < 40)                  { gfx_printf("bmp: biSize=%u\n", (unsigned)ih.biSize); file_close(f); return false; }
    if (ih.biPlanes != 1)                { gfx_printf("bmp: planes=%u\n", (unsigned)ih.biPlanes); file_close(f); return false; }
    if (ih.biBitCount != 16)             { gfx_printf("bmp: bpp=%u\n", (unsigned)ih.biBitCount); file_close(f); return false; }

    int w = ih.biWidth;
    int h = ih.biHeight;
    bool bottom_up = true;
    if (h < 0) { h = -h; bottom_up = false; }

    // BITFIELDS-Masken ggf. konsumieren (aber NICHT abbrechen, nur debuggen)
    uint32_t rmask = 0, gmask = 0, bmask = 0;
    if (ih.biCompression == BI_BITFIELDS) {
        if (!read_exact(f, &rmask, 4) || !read_exact(f, &gmask, 4) || !read_exact(f, &bmask, 4)) {
            gfx_printf("bmp: read masks failed\n");
            file_close(f); return false;
        }
        gfx_printf("bmp: masks r=%x g=%x b=%x\n", rmask, gmask, bmask);
    } else if (ih.biCompression != BI_RGB) {
        gfx_printf("bmp: unsupported comp=%u\n", (unsigned)ih.biCompression);
        file_close(f); return false;
    }

    // Skip bis bfOffBits (sequenziell)
    uint32_t cur = (uint32_t)sizeof(BMP_FILEHDR) + (uint32_t)sizeof(BMP_INFOHDR);
    if (ih.biCompression == BI_BITFIELDS) cur += 12;

    if (fh.bfOffBits < cur) {
        gfx_printf("bmp: offBits(%u) < cur(%u)\n", fh.bfOffBits, cur);
        file_close(f); return false;
    }

    uint32_t skip = fh.bfOffBits - cur;
    while (skip) {
        uint8_t tmp[64];
        uint32_t n = (skip > sizeof(tmp)) ? (uint32_t)sizeof(tmp) : skip;
        if (!read_exact(f, tmp, n)) { gfx_printf("bmp: skip failed\n"); file_close(f); return false; }
        skip -= n;
    }

    // *** WICHTIG: 4-Byte aligned stride ***
    uint32_t bmp_stride = (uint32_t)((w * 2 + 3) & ~3);
    gfx_printf("bmp: stride=%u pitch=%u\n", bmp_stride, pitch);

    uint8_t* row = (uint8_t*)kmalloc(bmp_stride);
    if (!row) { gfx_printf("bmp: kmalloc failed\n"); file_close(f); return false; }

    for (int y = 0; y < h; y++) {
        if (!read_exact(f, row, bmp_stride)) { gfx_printf("bmp: row read fail y=%d\n", y); kfree(row); file_close(f); return false; }

        int out_y = bottom_up ? (h - 1 - y) : y;
        int sy = dst_y + out_y;
        if (sy < 0 || sy >= screen_h) continue;

        uint16_t* dst_line = (uint16_t*)(lfb + (uint32_t)sy * pitch);

        // Clip X
        int x0 = 0, x1 = w;
        int dx = dst_x;
        if (dx < 0) { x0 = -dx; dx = 0; }
        if (dx + (x1 - x0) > screen_w) x1 = x0 + (screen_w - dx);
        if (x0 >= x1) continue;

        const uint16_t* src = (const uint16_t*)row;

        // 565 -> 565 kopieren (wenn deine BMP in Wahrheit 555 ist, sind Farben “off”, aber es wird zumindest gezeichnet)
        for (int x = x0; x < x1; x++) {
            dst_line[dx + (x - x0)] = src[x];
        }
    }

    kfree(row);
    file_close(f);
    return true;
}
