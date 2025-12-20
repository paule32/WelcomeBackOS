# include "stdint.h"
# include "proto.h"

# define DESKTOP
# include "vga.h"

extern "C" void shell_main(void)
{
    USHORT red, green, blue;
    
    red   = gfx_rgbColor(255, 0, 0);
    green = gfx_rgbColor(0, 255, 0);
    blue  = gfx_rgbColor(0, 0, 255);

    // drei Pixel zeichnen
    gfx_putPixel(100, 50, red  );
    gfx_putPixel(101, 50, green);
    gfx_putPixel(102, 50, blue );

    for (int y = 1; y < 50; y++) {
        for (int x = 1; x < 20; x++) {
            gfx_putPixel(120+x, 150+y, green);
        }
    }

    // dünne Linie
    gfx_drawLine(50,  50, 300, 100, 1, red);
    // mitteldick
    gfx_drawLine(50, 150, 300, 250, 3, green);
    // sehr dick
    gfx_drawLine(50, 300, 300, 450, 7, blue);
    
    red   = gfx_rgbColor(255, 0,   0);
    green = gfx_rgbColor(0,   255, 0);
    blue  = gfx_rgbColor(0,   0,   255);

    gfx_drawLine(50,  50, 300, 100, 1, red  );  // dünn
    gfx_drawLine(50, 150, 300, 250, 4, green);  // dicker
    gfx_drawLine(50, 300, 300, 450, 8, blue );  // sehr dick

    gfx_rectFill(300, 300, 100, 42, green);
    
    gfx_rectFill (50, 50, 300, 200,    gfx_rgbColor(0  , 120, 255));  // Block
    gfx_rectFrame(50, 50, 300, 200, 4, gfx_rgbColor(255, 255, 255));  // Rahmen
    
    
    // dünn
    gfx_drawCircle(200, 150, 50, 1, red);
    // mittel
    gfx_drawCircle(400, 300, 80, 4, green);
    // sehr dick
    gfx_drawCircle(600, 400, 60, 8, blue);
    
    gfx_drawCircle(200, 200, 60, 8, blue);
    gfx_drawCircleFill(260, 200, 50, red);
    
    gfx_rectFill (50, 50, 300, 100,  gfx_rgbColor(0, 120, 255));  // Block
    
    gfx_rectFill (350, 350, 200, 100,    gfx_rgbColor(0  , 120, 255));  // Block
    gfx_rectFill (30,  350, 120, 100,    gfx_rgbColor(140, 120, 255));  // Block
    gfx_drawLine (50,   50, 300, 100, 1, gfx_rgbColor(255, 0, 0));
    
    gfx_rectFill (30, 350, 20, 100,    gfx_rgbColor(40  , 120, 255));  // Block
    gfx_rectFill (50, 350, 20, 100,    gfx_rgbColor(40  , 120, 255));  // Block
    
    
    gfx_putPixel(100, 50, red  );
    gfx_putPixel(101, 50, green);
    gfx_putPixel(102, 50, blue );

    gfx_rectFill (350, 350, 200, 100,    gfx_rgbColor(0  , 120, 255));  // Block
    gfx_rectFill (230, 350, 120, 100,    gfx_rgbColor(140, 120, 255));  // Block
    gfx_drawLine (250,  50, 300, 100, 1, gfx_rgbColor(255, 0, 0));
    
    gfx_drawCircle(260, 200, 50, red);
}
