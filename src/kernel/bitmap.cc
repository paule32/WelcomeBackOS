# include "stdint.h"
# include "kheap.h"
# include "iso9660.h"

# define DESKTOP
# include "vga.h"
# include "bitmap.h"

// ---- BMP constants ----
#define BI_RGB       0
#define BI_BITFIELDS 3

extern uint32_t  file_read (FILE* f, void* buf, uint32_t len);
extern int       file_seek (FILE* f, uint32_t new_pos);
extern void      file_close(FILE* f);

# define BI_RGB       0
# define BI_BITFIELDS 3

# define KEY565 gfx_rgbColor(255,255,255)

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

typedef struct __attribute__((packed)) {
    uint8_t b, g, r, a;       // BMP Palette: B,G,R,0
} RGBQUAD;

typedef struct {
    int w, h;
    uint32_t pitch;     // bytes per row
    uint16_t* pixels;   // RGB565
} sprite565_t;

typedef unsigned int addr_t; // 32-bit

static inline uint16_t bgr_to_565(uint8_t b, uint8_t g, uint8_t r) {
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}
static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static bool read_exact(FILE* f, void* buf, uint32_t len) {
    uint32_t got = file_read(f, buf, len);
    return got == len;
}

static bool skip_bytes(FILE* f, uint32_t count)
{
    uint8_t buf[64];

    while (count > 0) {
        uint32_t chunk = (count > sizeof(buf)) ? (uint32_t)sizeof(buf) : count;

        // exakt chunk Bytes lesen und verwerfen
        if (file_read(f, buf, chunk) != chunk)
            return false;

        count -= chunk;
    }
    return true;
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

// src: 16bpp RGB565 sprite, mit src_pitch bytes pro Zeile (oft w*2, kann aber gepaddet sein)
// dst: LFB 16bpp RGB565, mit dst_pitch bytes pro Zeile (VBE pitch!)
extern "C" void blit565_colorkey(
    uint8_t* dst, uint32_t dst_pitch,
    int dst_w, int dst_h,
    int dst_x, int dst_y,
    const uint16_t* src,
    uint32_t src_pitch,
    int src_w, int src_h,
    uint16_t key565)
{
    // Clip Y
    int y0 = 0, y1 = src_h;
    if (dst_y < 0) { y0 = -dst_y; dst_y = 0; }
    if (dst_y + (y1 - y0) > dst_h) y1 = y0 + (dst_h - dst_y);
    if (y0 >= y1) return;

    // Clip X
    int x0 = 0, x1 = src_w;
    if (dst_x < 0) { x0 = -dst_x; dst_x = 0; }
    if (dst_x + (x1 - x0) > dst_w) x1 = x0 + (dst_w - dst_x);
    if (x0 >= x1) return;

    for (int y = y0; y < y1; y++) {
        uint16_t* d = (uint16_t*)(dst + (uint32_t)(dst_y + (y - y0)) * dst_pitch) + dst_x;
        const uint16_t* s = (const uint16_t*)((const uint8_t*)src + (uint32_t)y * src_pitch) + x0;

        for (int x = x0; x < x1; x++) {
            uint16_t p = *s++;
            if (p != key565) *d = p;   // nur nicht-Key-Pixel zeichnen
            d++;
        }
    }
}

extern "C" bool bmp_load_16bpp565_to_sprite(const char* path, sprite565_t* out)
{
    if (!out) return false;
    out->w = out->h = 0;
    out->pitch = 0;
    out->pixels = 0;

    FILE* f = file_open(path);
    if (!f) return false;

    BMP_FILEHDR fh;
    BMP_INFOHDR  ih;

    if (!read_exact(f, &fh, sizeof(fh))) { file_close(f); return false; }
    if (fh.bfType != 0x4D42)             { file_close(f); return false; } // "BM"
    if (!read_exact(f, &ih, sizeof(ih))) { file_close(f); return false; }

    if (ih.biSize < 40)                  { file_close(f); return false; }
    if (ih.biPlanes != 1)                { file_close(f); return false; }
    if (ih.biBitCount != 16)             { file_close(f); return false; }

    int w = ih.biWidth;
    int h = ih.biHeight;

    if (w <= 0 || h == 0) { file_close(f); return false; }

    bool bottom_up = true;
    if (h < 0) { h = -h; bottom_up = false; }

    // Wenn BI_BITFIELDS: Masken lesen und 565 prüfen
    if (ih.biCompression == BI_BITFIELDS) {
        uint32_t rmask, gmask, bmask;
        if (!read_exact(f, &rmask, 4) || !read_exact(f, &gmask, 4) || !read_exact(f, &bmask, 4)) {
            file_close(f); return false;
        }
        if (!(rmask == 0xF800 && gmask == 0x07E0 && bmask == 0x001F)) {
            // Nicht RGB565 (z.B. 555 oder was anderes)
            file_close(f);
            return false;
        }
    } else if (ih.biCompression != BI_RGB) {
        file_close(f);
        return false;
    }

    // Wir stehen jetzt bei:
    // 14 + 40 (+12 falls BITFIELDS)
    uint32_t cur = (uint32_t)sizeof(BMP_FILEHDR) + (uint32_t)sizeof(BMP_INFOHDR);
    if (ih.biCompression == BI_BITFIELDS) cur += 12;

    if (fh.bfOffBits < cur) { file_close(f); return false; }
    if (!skip_bytes(f, fh.bfOffBits - cur)) { file_close(f); return false; }

    // BMP-Strides sind 4-Byte aligned!
    uint32_t bmp_stride = (uint32_t)(((uint32_t)w * 2 + 3) & ~3u);

    // Sprite: tight packed 565
    uint32_t spr_pitch = (uint32_t)w * 2u;
    uint32_t total = spr_pitch * (uint32_t)h;

    uint16_t* pixels = (uint16_t*)kmalloc(total);
    if (!pixels) { file_close(f); return false; }

    uint8_t* row = (uint8_t*)kmalloc(bmp_stride);
    if (!row) { kfree(pixels); file_close(f); return false; }

    for (int y = 0; y < h; y++) {
        if (!read_exact(f, row, bmp_stride)) {
            kfree(row);
            kfree(pixels);
            file_close(f);
            return false;
        }

        int out_y = bottom_up ? (h - 1 - y) : y;
        uint16_t* dst = (uint16_t*)((uint8_t*)pixels + (uint32_t)out_y * spr_pitch);

        // row enthält mindestens w*2 Bytes Nutzdaten
        const uint16_t* src = (const uint16_t*)row;

        // 565 -> 565 kopieren
        for (int x = 0; x < w; x++) {
            dst[x] = src[x];
        }
    }

    kfree(row);
    file_close(f);

    out->w = w;
    out->h = h;
    out->pitch = spr_pitch;
    out->pixels = pixels;
    return true;
}

extern "C" bool show_bmp_with_germany_flag(
    const char* bmp_path,
    const char* flag_path,
    uint8_t* lfb, uint32_t pitch,
    int screen_w, int screen_h,
    int bmp_x, int bmp_y,
    int flag_x, int flag_y)
{
    if (!bmp_show_from_iso_16bpp565(bmp_path, lfb, pitch, screen_w, screen_h, bmp_x, bmp_y))
        return false;

    sprite565_t flag = {0};
    if (!bmp_load_16bpp565_to_sprite(flag_path, &flag))
        return false;

    blit565_colorkey(
        lfb, pitch, screen_w, screen_h,
        flag_x, flag_y,
        flag.pixels, flag.pitch,
        flag.w, flag.h,
        KEY565
    );

    // flag.pixels danach freigeben, falls per kmalloc
    // kfree(flag.pixels);

    return true;
}
