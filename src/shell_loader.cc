# include "stdint.h"
# include "proto.h"
# include "int86.h"
# include "iso9660.h"

# define DESKTOP
# include "vga.h"
# include "wm.h"

extern int  gfx_init(void);
extern "C" void shell_main(void);
extern "C" void mouse_poll(void);
extern "C"  int mouse_install(void);
//extern void wm_init(int,int,uint32_t,int);

typedef void (*app_entry_t)(void);

static inline void io_wait(void) { outb(0x80, 0); }

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

extern "C" void enter_shell(void)
{
    mouse_install();
    shell_main();

REGS16 in = {0}, out = {0};
in.ax = 0x0003;
gfx_printf("before\n");
//int rc = int86(0x10, &in, &out);
gfx_printf("after\n");

    /*
    wm_init(
    lfb_xres, lfb_yres,
    lfb_base, lfb_pitch);

    wm_create_window( 60,  60, 320, 200, "Console");
    wm_create_window(120, 120, 280, 160, "Test"   );*/

    // einmal initial zeichnen
    //wm_tick();
    
    ps2_polling_enable();
    //asm volatile("sti");      // Timer/Rest läuft weiter    

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
