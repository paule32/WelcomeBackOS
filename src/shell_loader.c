# include "stdint.h"
# include "proto.h"
# include "iso9660.h"
# include "vga.h"

extern int  gfx_init(void);
extern void shell_main(void);

typedef void (*app_entry_t)(void);

void enter_shell(void)
{
    shell_main();
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
