// --------------------------------------------------------------------------
// \file   vga.h
// \author Jens Kallup - paule32
// \note   (c) 2025 all rights reserved.
// --------------------------------------------------------------------------
#ifndef __VGA_H__
#define __VGA_H__

# include "stdint.h"

#ifndef DESKTOP
  #ifdef __cplusplus
  extern "C" {
  #endif // __cplusplus
#endif

typedef struct
{
    uint16_t attributes;      // 0x00
    uint8_t  winA;            // 0x02
    uint8_t  winB;            // 0x03
    uint16_t granularity;     // 0x04
    uint16_t winsize;         // 0x06
    uint16_t segmentA;        // 0x08
    uint16_t segmentB;        // 0x0A
    uint32_t realFctPtr;      // 0x0C
    uint16_t pitch;           // 0x10 (Bytes pro Scanline)
    
    uint16_t xres;          // Offset 0x12: XResolution
    uint16_t yres;          // Offset 0x14: YResolution
    uint8_t  x_char_size;   // 0x16
    uint8_t  y_char_size;   // 0x17
    uint8_t  planes;        // 0x18
    uint8_t  bpp;           // 0x19: BitsPerPixel
    uint8_t  banks;         // 0x1A
    uint8_t  memory_model;  // 0x1B
    uint8_t  bank_size;     // 0x1C
    uint8_t  image_pages;   // 0x1D
    uint8_t  reserved1;     // 0x1E

    uint8_t  red_mask_size;       // 0x1F
    uint8_t  red_field_position;  // 0x20
    uint8_t  green_mask_size;     // 0x21
    uint8_t  green_field_position;// 0x22
    uint8_t  blue_mask_size;      // 0x23
    uint8_t  blue_field_position; // 0x24
    uint8_t  rsvd_mask_size;      // 0x25
    uint8_t  rsvd_field_position; // 0x26
    uint8_t  direct_color_info;   // 0x27

    uint32_t phys_base;      // 0x28: PhysBasePtr (FrameBuffer-Adresse)
    uint32_t offscreen_off;  // 0x2C
    uint16_t offscreen_size; // 0x30

    //uint8_t  reserved2[206]; // auf 256 Bytes auffüllen
}   __attribute__((packed)) vbe_info_t;

extern volatile uint32_t lfb_base;

extern USHORT  lfb_pitch;
extern USHORT  lfb_xres ;
extern USHORT  lfb_yres ;
extern UCHAR   lfb_bpp  ;

#ifdef DESKTOP
extern   void gfx_clear          (                    USHORT);
#ifdef __cplusplus
extern   void gfx_drawCircle     (int,int,int,int,    USHORT);
#endif
extern   void gfx_drawCircle     (int,int,int,        USHORT);
extern   void gfx_drawCircleFill (int,int,int,        USHORT);
extern   void gfx_drawLine       (int,int,int,int,int,USHORT);
extern   void gfx_drawChar       (int,int,uint8_t,    USHORT,USHORT);
extern   void gfx_hLine          (int,int,int,        USHORT);
extern USHORT gfx_getPixel       (int,int);
extern   void gfx_putPixel       (int,int,            USHORT);
#ifdef __cplusplus
extern   void gfx_putPixel       (int,int,int,        USHORT);
#endif
extern   void gfx_rectFill       (int,int,int,int,    USHORT);
extern   void gfx_rectFrame      (int,int,int,int,int,USHORT);
extern   void gfx_print          (const char*);
extern   void gfx_printf         (char*, ... );
extern   void gfx_putChar        (char);

extern USHORT gfx_rgbColor(UCHAR,UCHAR,UCHAR);
#else
extern    int gfx_init(void);
#endif // __cplusplus

#ifndef DESKTOP
  #ifdef __cplusplus
  };
  #endif // __cplusplus
#endif   // DESKTOP

#endif   // __VGA_H__
