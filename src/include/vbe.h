#ifndef __VBE_H__
#define __VBE_H__

extern volatile UCHAR* lfb_base;
extern USHORT          lfb_pitch;
extern USHORT          lfb_xres;
extern USHORT          lfb_yres;
extern UCHAR           lfb_bpp;
extern ULONG           lfb_phys;

//extern page_directory_t* kernel_directory;
//extern page_t* get_page(ULONG addr, int make, page_directory_t* dir);

#endif
