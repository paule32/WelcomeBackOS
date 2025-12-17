# include "stdint.h"
# include "proto.h"
# include "vga.h"

void shell_main(void)
{
    // Testmarker, bevor wir springen:
    volatile char* VGA = (volatile char*)0xB8000;
    VGA[0] = 'K'; VGA[1] = 0x0F;
    VGA[2] = 'U'; VGA[3] = 0x0F;
    
    printformat("lolo\n");
    
    
    /*
    gfx_init();
    
    gfx_rectFill(20,20,100,100,rgb565(200,200,100));
    gfx_putPixel(100,100, rgb565(200,200,100));*/
}
