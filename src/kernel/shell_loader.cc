// ----------------------------------------------------------------------------
// \file  shell_loader.cc
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "config.h"

# include "stdint.h"
# include "proto.h"
# include "kheap.h"
# include "int86.h"
# include "iso9660.h"
# include "elf_loader.h"

# define DESKTOP
# include "vga.h"
# include "wm.h"
# include "bitmap.h"

extern int  gfx_init(void);
extern "C" void shell_main(void);
extern "C" void mouse_poll(void);
extern "C"  int mouse_install(void);
extern "C" void test_app(void);

//extern void wm_init(int,int,uint32_t,int);

typedef struct {
    int w, h;
    uint32_t pitch;     // bytes per row
    uint16_t* pixels;   // RGB565
} sprite565_t;

extern "C" bool bmp_show_from_iso_16bpp565(
    const char* path,
    uint8_t* lfb,
    uint32_t pitch,
    int screen_w, int screen_h,
    int dst_x, int dst_y);
extern "C" bool bmp_load_16bpp565_to_sprite(
    const char* path,
    sprite565_t* out);
extern "C" void blit565_colorkey(
    uint8_t* dst, uint32_t dst_pitch,
    int dst_w, int dst_h,
    int dst_x, int dst_y,
    const uint16_t* src,
    uint32_t src_pitch,
    int src_w, int src_h,
    uint16_t key565);
    
typedef void (*app_entry_t)(void);

static inline void io_wait(void) { outb(0x80, 0); }
extern "C" bool elf32_load_nomap(FILE* f, uint32_t base_for_dyn, elf_user_image_t* out);

static void pic_mask_irq(uint8_t irq)
{
    uint16_t port = (irq < 8) ? 0x21 : 0xA1;
    uint8_t  bit  = (irq < 8) ? irq : (irq - 8);
    outb(port, inb(port) | (1u << bit));
}

void ps2_polling_enable(void)
{
    pic_mask_irq(1);   // Keyboard IRQ1
    pic_mask_irq(12);  // Mouse IRQ12
}

void enter_txt_shell(void)
{
    printformat("in text shell.\n");
    test_app();
    /*
    FILE *user_file = file_open("/shell32.exe");
    if (!user_file) {
        printformat("shell32.exe: not found.\n");
        return;
    }*/
    
    /*if (!elf32_load_nomap(user_file, 2, nullptr)) {
        file_close(user_file);
        printformat("textshell program error.\n");
    }*/
}

void enter_vid_shell(void)
{
    mouse_install();
    //shell_main();

    /*
    wm_init(
    lfb_xres, lfb_yres,
    lfb_base, lfb_pitch);

    wm_create_window( 60,  60, 320, 200, "Console");
    wm_create_window(120, 120, 280, 160, "Test"   );*/

    // einmal initial zeichnen
    //wm_tick();
    
    bmp_show_from_iso_16bpp565(
        "/img/lo.bmp",
        (uint8_t*)lfb_base, lfb_pitch,
        lfb_xres, lfb_yres, 0, 0);
    
    sprite565_t* flag = (sprite565_t*)kmalloc(sizeof(sprite565_t));
    if (bmp_load_16bpp565_to_sprite("/img/flag_us.bmp", flag)) {
        blit565_colorkey(
        (uint8_t*)lfb_base, lfb_pitch,
        lfb_xres,lfb_yres,
        560,160,
        flag->pixels,
        flag->pitch,
        flag->w,
        flag->h,
        gfx_rgbColor(0,0,0));
        kfree(flag->pixels);
    }
    if (bmp_load_16bpp565_to_sprite("/img/sel_us_1.bmp", flag)) {
        blit565_colorkey(
        (uint8_t*)lfb_base, lfb_pitch,
        lfb_xres,lfb_yres,
        560,245,
        flag->pixels,
        flag->pitch,
        flag->w,
        flag->h,
        gfx_rgbColor(0,0,0));
        kfree(flag->pixels);
    }

    
    if (bmp_load_16bpp565_to_sprite("/img/flag_de.bmp", flag)) {
        blit565_colorkey(
        (uint8_t*)lfb_base, lfb_pitch,
        lfb_xres,lfb_yres,
        560,360,
        flag->pixels,
        flag->pitch,
        flag->w,
        flag->h,
        gfx_rgbColor(255,255,255));
        kfree(flag->pixels);
    }
    if (bmp_load_16bpp565_to_sprite("/img/sel_de_1.bmp", flag)) {
        blit565_colorkey(
        (uint8_t*)lfb_base, lfb_pitch,
        lfb_xres,lfb_yres,
        560,480,
        flag->pixels,
        flag->pitch,
        flag->w,
        flag->h,
        gfx_rgbColor(0,0,0));
        kfree(flag->pixels);
    }
    
    gfx_rectFill(100, 100, 200, 40, gfx_rgbColor(200,100,200));
    
    
    ps2_polling_enable();
    //asm volatile("sti");      // Timer/Rest läuft weiter    

    wm_init(
    lfb_xres, lfb_yres,
    lfb_base, lfb_pitch);

    wm_create_window( 60,  60, 320, 200, "Console");
    //wm_create_window(120, 120, 280, 160, "Test"   );
    
    wm_tick();
    for (;;) {
        mouse_poll();
        
        //wm_tick();
        io_wait();
    }

    for(;;);
    
    /*
    FILE* f = file_open("/shell.exe");
    if (!f) {
        printformat("ERROR: shell.exe not found.\n");
        for (;;);
    }   else {
        // Datei lesen, z.B. in Buffer und als PE an Loader weitergeben
        uint8_t* buffer = (uint8_t*)kmalloc(f->size);
        
        file_read(f, buffer, f->size);
        file_close(f);
        
        app_entry_t entry = (app_entry_t)buffer;
        gfx_init();
        
        //entry();
        shell_main();
        for(;;);
    }
    */
}


extern "C" void enter_shell(void)
{
    if (graph_mode == 1) {
        enter_txt_shell();
    }   else {
        enter_vid_shell();
    }
}
