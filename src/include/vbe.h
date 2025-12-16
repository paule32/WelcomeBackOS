// --------------------------------------------------------------------------
// \file   vbe.h
// \author Jens Kallup - paule32
// \note   (c) 2025 all rights reserved.
// --------------------------------------------------------------------------
#ifndef __VBE_H__
#define __VBE_H__

# include "stdint.h"

typedef struct {
    uint32_t sig;        // 'VBEI'
    uint16_t mode;
    uint16_t xres;
    uint16_t yres;
    uint8_t  bpp;
    uint8_t  pitch;
    uint8_t  reserved;
    uint32_t phys_base;  // physische Adresse des LFB
}   __attribute__((packed)) vbe_info_t;

# define VBE_MODE_INFO_PTR ((const vbe_info_t*)0x00000A00)

extern volatile UCHAR* lfb_base;

extern USHORT          lfb_pitch;
extern USHORT          lfb_xres;
extern USHORT          lfb_yres;
extern UCHAR           lfb_bpp;
extern ULONG           lfb_phys;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern void gfx_init(void);
extern void gfx_rectFill(int,int,int,int,USHORT);

extern USHORT rgb565(UCHAR,UCHAR,UCHAR);

#ifdef __cplusplus
};
#endif // __cplusplus
#endif // __VBE_H__
