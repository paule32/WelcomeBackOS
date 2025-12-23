# include "stdint.h"
# include "kheap.h"
# include "iso9660.h"

# define DESKTOP
# include "vga.h"
# include "bitmap.h"

extern uint32_t file_read (FILE* f, void* buf, uint32_t len);
extern int      file_seek (FILE* f, uint32_t new_pos);
extern void     file_close(FILE* f);

typedef struct __attribute__((packed)) {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfRes1;
    uint16_t bfRes2;
    uint32_t bfOffBits;
} BMP_FILEHDR;

typedef struct __attribute__((packed)) {
    uint32_t biSize;        // 124 (V5) bei deinem lo.bmp
    int32_t  biWidth;
    int32_t  biHeight;      // >0 bottom-up, <0 top-down
    uint16_t biPlanes;      // 1
    uint16_t biBitCount;    // 16
    uint32_t biCompression; // 3 (BI_BITFIELDS)
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;

    uint32_t bV5RedMask;    // 0xF800
    uint32_t bV5GreenMask;  // 0x07E0
    uint32_t bV5BlueMask;   // 0x001F
    uint32_t bV5AlphaMask;
} BMP_INFO_V5_PREFIX;

static uint32_t bmp_pos = 0;
static bool read_exact1(
    FILE* f,
    BMP_FILEHDR* buffer,
    uint32_t len) {
    uint32_t n = 0;
    
    gfx_printf("header size: %d\n",sizeof(BMP_FILEHDR));
    file_seek(f, 0);
    n = file_read(f, (BMP_FILEHDR*)buffer, sizeof(BMP_FILEHDR));
    if (n != sizeof(BMP_FILEHDR)) {
        gfx_printf("read file header: fail.\n");
        return false;
    }   else {
        bmp_pos += n;
        gfx_printf("read file header: success.\n");
    }
    file_seek(f,0);
    if (buffer->bfType != 0x4D42) {
        gfx_printf("file is not bitmap: 0x%x != 0x4D42.\n", buffer->bfType);
        return false;
    }
    return true;
}
static bool read_exact2(
    FILE* f,
    BMP_INFO_V5_PREFIX* buffer,
    uint32_t len) {
    uint32_t n = 0;
    
    gfx_printf("header size: %d\n",sizeof(BMP_INFO_V5_PREFIX));
    file_seek(f,sizeof(BMP_FILEHDR));
    n = file_read(f, (BMP_INFO_V5_PREFIX*)buffer, sizeof(BMP_INFO_V5_PREFIX));
    if (n != sizeof(BMP_INFO_V5_PREFIX)) {
        gfx_printf("read file header: fail.\n");
        return false;
    }   else {
        gfx_printf("read info: success.\n");
    }
    return true;
}
static bool read_exact(
    FILE* f,
    uint8_t* buffer,
    uint32_t len) {
}
static bool skip_bytes(FILE *f, uint32_t n)
{
    uint8_t tmp[128];
    while (n) {
        uint32_t chunk = (n > sizeof(tmp)) ? sizeof(tmp) : n;
        if (file_read(f, tmp, chunk) != chunk) return false;
        n -= chunk;
    }
    return true;
}
static inline void lfb_put565(uint8_t *lfb, uint32_t pitch, int x, int y, uint16_t c)
{
    *(uint16_t*)(lfb + (uint32_t)y * pitch + (uint32_t)x * 2u) = c;
}
extern "C" bool bmp_show_from_iso_16bpp565(
    const char* path,
    uint8_t* lfb,
    uint32_t pitch,
    int screen_w, int screen_h,
    int dst_x, int dst_y) {
       
    FILE* f = file_open(path);
    if (!f) return false;
    gfx_printf("image loaded.\n");
    
    BMP_FILEHDR *fh = (BMP_FILEHDR*)kmalloc(sizeof(BMP_FILEHDR));
    if (!read_exact1(f, (BMP_FILEHDR*)fh, sizeof(fh))) { file_close(f); return false; }
    gfx_printf("fh->bfType: 0x%x == 0x%x ?\n", fh->bfType, 0x4D42);

    BMP_INFO_V5_PREFIX *ih = (BMP_INFO_V5_PREFIX*)kmalloc(sizeof(BMP_INFO_V5_PREFIX));
    if (!read_exact2(f, (BMP_INFO_V5_PREFIX*)ih, sizeof(BMP_INFO_V5_PREFIX))) { file_close(f); return false; }

    // Debug (wenn du Ausgabe hast):
    gfx_printf("BMP w=%d h=%d bpp=%d comp=%d off=%d size=%d\n",
         ih->biWidth,
         ih->biHeight,
         ih->biBitCount,
         ih->biCompression,
         fh->bfOffBits,
         ih->biSize);

    if (ih->biPlanes != 1)                { file_close(f); return false; }
    if (ih->biBitCount != 16)             { file_close(f); return false; }
    if (ih->biCompression != 3)           { file_close(f); return false; } // BI_BITFIELDS
    if (ih->bV5RedMask != 0xF800u ||
        ih->bV5GreenMask != 0x07E0u ||
        ih->bV5BlueMask != 0x001Fu)       { file_close(f); return false; } // muss 565 sein

    int w = ih->biWidth;
    int h = ih->biHeight;
    bool bottom_up = true;
    if (h < 0) { bottom_up = false; h = -h; }

    // BMP stride: width*2, auf 4 Bytes gerundet
    uint32_t bmp_stride = ((uint32_t)w * 2u + 3u) & ~3u;

    // WICHTIG: bis zu den Pixeln vorspulen (ohne seek)
    // Wir haben schon: FILEHDR + sizeof(ih) gelesen.
    uint32_t already = sizeof(BMP_FILEHDR) + (uint32_t)sizeof(BMP_INFO_V5_PREFIX);
    if (fh->bfOffBits < already)          { file_close(f); return false; }
    if (!skip_bytes(f, fh->bfOffBits - already)) { file_close(f); return false; }

    // lo.bmp braucht 508 Bytes pro Zeile; 2048 reicht locker
    if (bmp_stride > 2048)               { file_close(f); return false; }
    uint8_t rowbuf[2048];

    for (int y = 0; y < h; y++) {
        if (!read_exact(f, rowbuf, bmp_stride)) { file_close(f); return false; }

        int out_y = bottom_up ? (h - 1 - y) : y;
        int sy = dst_y + out_y;
        if ((unsigned)sy >= (unsigned)screen_h) continue;

        const uint16_t* src = (const uint16_t*)rowbuf;

        // Clipping X (damit dst_x auch negativ sein darf)
        int start_x = 0;
        int copy_w  = w;

        if (dst_x < 0) { start_x = -dst_x; copy_w -= start_x; }
        if (dst_x + copy_w > screen_w) copy_w = screen_w - dst_x;
        if (copy_w <= 0) continue;

        uint16_t* dst = (uint16_t*)(lfb + (uint32_t)sy * lfb_pitch
                                         + (uint32_t)(dst_x + start_x) * 2u);

        for (int x = 0; x < copy_w; x++) {
            dst[x] = src[start_x + x]; // 565 -> 565
        }
    }
gfx_printf("close image.\n");
    file_close(f);
    return true;
}
