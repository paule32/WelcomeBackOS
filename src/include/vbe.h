#ifndef __VBE_H__
#define __VBE_H__

extern volatile UCHAR* lfb_base;
extern USHORT          lfb_pitch;
extern USHORT          lfb_xres;
extern USHORT          lfb_yres;
extern UCHAR           lfb_bpp;
extern ULONG           lfb_phys;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern void gfx_rectFill(int,int,int,int,USHORT);
extern USHORT rgb565(UCHAR,UCHAR,UCHAR);

#ifdef __cplusplus
};
#endif // __cplusplus
#endif // __VBE_H__
