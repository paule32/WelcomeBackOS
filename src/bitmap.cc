# include "stdint.h"
# include "iso9660.h"

# define DESKTOP
# include "vga.h"
# include "bitmap.h"

extern uint32_t file_read (FILE* f, void* buf, uint32_t len);
extern int      file_seek (FILE* f, uint32_t new_pos);
extern void     file_close(FILE* f);

// ---------- kleine Helpers ----------
static inline uint16_t rgb888_to_565(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}
static uint32_t file_pread(FILE* f, uint32_t off, uint32_t len, void* buf) {
    file_seek(f, off);
    return file_read(f, buf, len);
}
// ---------- Anzeige-Funktion ----------
int bmp_show_from_iso(
    const char *path,
    int dst_x,
    int dst_y) {
        
    FILE *f = file_open(path);
    if (!f) return 0;

    BMPFileHeader fh;
    BMPInfoHeader ih;

    if (file_pread(f, 0, sizeof(fh), (uint8_t*)&fh) != (int)sizeof(fh)) { file_close(f); return 0; }
    if (file_pread(f, sizeof(fh), sizeof(ih), (uint8_t*)&ih) != (int)sizeof(ih)) { file_close(f); return 0; }

    if (fh.bfType != 0x4D42) { file_close(f); return 0; }           // "BM"
    if (ih.biSize < 40)      { file_close(f); return 0; }           // mindestens BITMAPINFOHEADER
    if (ih.biPlanes != 1)    { file_close(f); return 0; }
    if (ih.biCompression != 0) { file_close(f); return 0; }         // nur BI_RGB
    if (!(ih.biBitCount == 24 || ih.biBitCount == 32)) { file_close(f); return 0; }

    int w = ih.biWidth;
    int h = ih.biHeight;
    int top_down = 0;
    if (h < 0) { h = -h; top_down = 1; }

    if (w <= 0 || h <= 0) { file_close(f); return 0; }

    // Bytes pro Pixel und Row-Stride (BMP rows sind 4-byte aligned)
    int bpp_bytes = ih.biBitCount / 8;
    uint32_t row_raw = (uint32_t)w * (uint32_t)bpp_bytes;
    uint32_t row_stride = (row_raw + 3u) & ~3u;

    // Row-Buffer (auf Stack ok bei 800*4=3200 Bytes; sonst statisch/heap)
    static uint8_t rowbuf[4096]; // reicht bis ~1024px @ 32bpp (1024*4=4096)
    if (row_stride > sizeof(rowbuf)) { file_close(f); return 0; }

    // Zeichnen (cropping wenn nötig)
    int max_w = w;
    int max_h = h;
    if (dst_x + max_w > (int)lfb_xres) max_w = (int)lfb_xres - dst_x;
    if (dst_y + max_h > (int)lfb_yres) max_h = (int)lfb_yres - dst_y;
    if (max_w <= 0 || max_h <= 0) { file_close(f); return 0; }

    for (int y = 0; y < max_h; y++)
    {
        // BMP: wenn bottom-up, dann kommt Zeile 0 im File = unterste Bildzeile
        int src_y = top_down ? y : (h - 1 - y);
        uint32_t off = fh.bfOffBits + (uint32_t)src_y * row_stride;

        if (file_pread(f, off, row_stride, rowbuf) != (int)row_stride) { file_close(f); return 0; }

        for (int x = 0; x < max_w; x++)
        {
            uint8_t b = rowbuf[x * bpp_bytes + 0];
            uint8_t g = rowbuf[x * bpp_bytes + 1];
            uint8_t r = rowbuf[x * bpp_bytes + 2];
            // 32bpp: rowbuf[x*4+3] wäre Alpha (meist ignorieren)
            gfx_putPixel(dst_x + x, dst_y + y, rgb888_to_565(r, g, b));
        }
    }

    file_close(f);
    return 1;
}
