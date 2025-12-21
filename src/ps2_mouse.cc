// ps2_mouse.c

# include "stdint.h"
# include "proto.h"

# define DESKTOP 1
# include "vga.h"

// Ports
#define PS2_DATA    0x60
#define PS2_STATUS  0x64
#define PS2_CMD     0x64

// Status bits
#define ST_OUT_FULL 0x01
#define ST_IN_FULL  0x02

#define CUR_W 16
#define CUR_H 16

static const uint16_t cursor_mask[CUR_H] = {
    0b1000000000000000,
    0b1100000000000000,
    0b1110000000000000,
    0b1111000000000000,
    0b1111100000000000,
    0b1111110000000000,
    0b1111111000000000,
    0b1111111100000000,
    0b1111111110000000,
    0b1111111111000000,
    0b1111110000000000,
    0b1110110000000000,
    0b1100011000000000,
    0b1000001100000000,
    0b0000000110000000,
    0b0000000010000000,
};

signed char mouse_cycle = 0;
signed char mouse_byte[ 3 ];

uint16_t mouse_x = 0;
uint16_t mouse_y = 0;

static int mx = 400;
static int my = 300;

static void cursor_draw_shape(int x, int y)
{
    uint16_t color = gfx_rgbColor(255,255,255);
    gfx_drawCircleFill(x,y,40,color);
}
void cursor_update(int x, int y)
{
    // alten Cursor entfernen
    //if (cur_old_x >= 0) {
    //    cursor_restore_bg(cur_old_x, cur_old_y);
    //}
//gfx_rectFill(500,100,100,100,gfx_rgbColor(100,200,100));
    // neuen Hintergrund sichern + Cursor zeichnen
    //cursor_save_bg(x, y);
    cursor_draw_shape(x, y);

//    cur_old_x = x;
    //cur_old_y = y;
}

static void ps2_drain_output_once(void) {
    if (inportb(PS2_STATUS) & ST_OUT_FULL)
        (void)inportb(PS2_DATA);
}

void mouse_loop(void)
{
    switch (mouse_cycle)
    {
        case 0:
            mouse_byte[0] = inportb(0x60);
            mouse_cycle++;
        break;
        case 1:
            mouse_byte[1] = inportb(0x60);
            mouse_cycle++;
        break;
        case 2:
            mouse_byte[2] = inportb(0x60);
            mouse_x       = mouse_byte[1];
            mouse_y       = mouse_byte[2];
            mouse_cycle   = 0;
        break;
    }
    cursor_update(mouse_x, mouse_y);
}

inline int mouse_wait(uint8_t a_type)
{
    uint32_t timeout = 500000;
    while (timeout--) {
        uint8_t st = inportb(PS2_STATUS);

        // Output-Buffer leeren, damit nichts staut
        if (st & ST_OUT_FULL) (void)inportb(PS2_DATA);

        if (a_type == 0) {          // warten auf Output
            if (st & ST_OUT_FULL) return 0;
        } else {                    // warten auf Input leer
            if ((st & ST_IN_FULL) == 0) return 0;
        }
    }
    return -1;
}

int mouse_write(uint8_t a_write)
{
    //Wait to be able to send a command
    if (mouse_wait(1) != 0) return -1;
    
    //Tell the mouse we are sending a command
    outportb(0x64, 0xD4);
    
    //Wait for the final part
    if (mouse_wait(1) != 0) return -1;
    //Finally write
    outportb(0x60, a_write);
    return 0;
}

static int mouse_try_read(uint8_t *out)
{
    uint8_t st = inb(0x64);
    if (st & 0x01) {            // Daten vorhanden ?
        if (st & 0x20) {        // von Maus ?
            uint8_t mb = inb(0x60);
            // mb is mouse byte
            gfx_rectFill(100,100,200,300,gfx_rgbColor(0,200,0));
        }   else {
            uint8_t kb = inb(0x60);
            // kb is keyboard byte
            gfx_rectFill(100,100,200,300,gfx_rgbColor(0,0,200));
        }
    }
    
    if ((st & 0x02) == 0) {
        *out = inb(0x60);
        return 1;
    }
    return 0;
}

extern "C" void mouse_poll(void)
{
    uint8_t b;
    if (!mouse_try_read(&b))
    return;

    // Sync: erstes Byte muss bit3=1 haben
    if (mouse_cycle == 0 && !(b & 0x08)) return;
    mouse_byte[mouse_cycle++] = b;

    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        int16_t dx = mouse_byte[1];
        int16_t dy = mouse_byte[2];

        mx += dx;
        my -= dy;

        if (mx < 0) mx = 0;
        if (my < 0) my = 0;
        
        if (mx >= lfb_xres) mx = lfb_xres - 1;
        if (my >= lfb_yres) my = lfb_yres - 1;

        cursor_update(mx, my);
    }
}

extern "C" void mouse_install()
{
    unsigned char _status;

    //Enable the auxiliary mouse device
    mouse_wait(1);
    outportb(0x64, 0xA8);

    //Enable the interrupts
    mouse_wait(1);
    outportb(0x64, 0x20);

    mouse_wait(0);
    _status = (inportb(0x60) | 2);
    
    mouse_wait(1);
    outportb(0x64, 0x60);
    
    mouse_wait(1);
    outportb(0x60, _status);
}
