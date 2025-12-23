#ifndef __BITMAP_H__
#define __BITMAP_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// ---------- BMP Header (packed) ----------
#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;      // 'BM' = 0x4D42
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;   // Pixel-Daten Offset
} BMPFileHeader;

typedef struct {
    uint32_t biSize;      // 40 bei BITMAPINFOHEADER
    int32_t  biWidth;
    int32_t  biHeight;    // >0: bottom-up, <0: top-down
    uint16_t biPlanes;    // 1
    uint16_t biBitCount;  // 24 oder 32 empfohlen
    uint32_t biCompression; // 0 = BI_RGB
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMPInfoHeader;
#pragma pack(pop)

int bmp_show_from_iso(
    const char *path,
    int dst_x,
    int dst_y);

#ifdef __cplusplus
};
#endif  // __cplusplus

#endif  // __BITMAP_H__
