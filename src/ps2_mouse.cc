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

static void ps2_flush_output(void) {
    for (int i=0;i<64;i++) {
        if (!(inb(PS2_STATUS) & ST_OUT_FULL)) break;
        (void)inb(PS2_DATA);
    }
}
static bool ps2_wait_in_clear(uint32_t t) {
    while (t--) if ((inb(PS2_STATUS) & ST_IN_FULL) == 0) return true;
    return false;
}
static int ps2_wait_out_full(uint32_t t) {
    while (t--) if (inb(0x64) & 0x01) return 1;
    return 0;
}
static int ps2_read_data(uint8_t *out) {
    if (!ps2_wait_out_full(200000)) return 0;
    *out = inb(0x60);
    return 1;
}
static bool ps2_send_cmd(uint8_t cmd) {
    if (!ps2_wait_in_clear(200000)) return false;
    outb(PS2_CMD, cmd);
    return true;
}
static bool ps2_write_data(uint8_t v) {
    if (!ps2_wait_in_clear(200000)) return false;
    outb(PS2_DATA, v);
    return true;
}
// an die Maus senden: erst 0xD4 an 0x64, dann Byte an 0x60
static bool ps2_mouse_write(uint8_t v) {
    if (!ps2_send_cmd(0xD4)) return false;
    return ps2_write_data(v);
}
static bool ps2_expect_ack(void) {
    uint8_t r;
    if (!ps2_read_data(&r)) return false;
    return (r == 0xFA); // ACK
}

static int mouse_try_read(uint8_t *out)
{
    uint8_t cfg;
    
    // 1) Controller Self-Test: 0xAA -> 0x55
    {
        // Alles, was im Output-Buffer hängt, weglesen
        for (int i = 0; i < 32; i++) {
            if (!(inb(0x64) & 0x01)) break;
            (void)inb(0x60);
        }

        uint32_t timeout = 200000;
        uint8_t  flag    = 0;
        while (timeout--) {
            if ((inb(0x64) & 0x02) == 0) {
                outb(0x64, 0xAA);
                flag = 1;
                break;
            }
        }
        if (flag == 0) {
            gfx_printf("PS2 Controller: timeout.\n");
            return 0;
        }   else {
            uint8_t st = inb(0x60);
            if (st == 0x55) {
                gfx_printf("PS/2 Controller: success.\n");
            }   else {
                gfx_printf("PS/2 Controller: error.\n");
                return 0;
            }
        }
    }
    
    // 2) Interface Test Port 1: 0xAB -> 0x00 (ok)
    {
        // Alles, was im Output-Buffer hängt, weglesen
        for (int i = 0; i < 32; i++) {
            if (!(inb(0x64) & 0x01)) break;
            (void)inb(0x60);
        }
        
        uint32_t timeout = 200000;
        uint8_t  flag    = 0;
        while (timeout--) {
            if ((inb(0x64) & 0x02) == 0) {
                outb(0x64, 0xAB);
                flag = 1;
                break;
            }
        }
        if (flag == 0) {
            gfx_printf("PS2 Controller: Port 1: timeout.\n");
            return 0;
        }   else {
            uint8_t st = inb(0x60);
            if (st == 0x00) {
                gfx_printf("PS/2 Port 1: success.\n");
            }   else {
                gfx_printf("PS/2 Port 1: error.\n");
                return 0;
            }
        }
    }
    // 3) Interface Test Port 2: 0xA9 -> 0x00 (ok)
    {
        // Alles, was im Output-Buffer hängt, weglesen
        for (int i = 0; i < 32; i++) {
            if (!(inb(0x64) & 0x01)) break;
            (void)inb(0x60);
        }
        
        uint32_t timeout = 200000;
        uint8_t  flag    = 0;
        while (timeout--) {
            if ((inb(0x64) & 0x02) == 0) {
                outb(0x64, 0xA9);
                flag = 1;
                break;
            }
        }
        if (flag == 0) {
            gfx_printf("PS2 Controller: Port 2: timeout.\n");
            return 0;
        }   else {
            uint8_t st = inb(0x60);
            if (st == 0x00) {
                gfx_printf("PS/2 Port 2: success.\n");
            }   else {
                gfx_printf("PS/2 Port 2: error.\n");
                return 0;
            }
        }
    }
    
    // AUX (Port 2) aktivieren
    {
        // Alles, was im Output-Buffer hängt, weglesen
        for (int i = 0; i < 32; i++) {
            if (!(inb(0x64) & 0x01)) break;
            (void)inb(0x60);
        }
        
        if (!ps2_send_cmd(0xA8)) {
            gfx_printf("PS2 Controller: AUX Port 2: timeout.\n");
            return 0;
        }   else {
            gfx_printf("PS2 Controller: AUX Port 2: active.\n");
        }
    }
    
    // Config Byte lesen
    {
        if (!ps2_send_cmd(0x20)) {
            gfx_printf("PS2 Controller: Config Byte: read error.\n");
            return 0;
        }   else {
            gfx_printf("PS2 Controller: Config Byte: read ok.\n");
        }
        if (!ps2_read_data(&cfg)) {
            gfx_printf("PS2 Controller: no data.\n");
            return 0;
        }   else {
            gfx_printf("PS2 Controller: have data.\n");
        }
    }
    
    // IRQ12 einschalten (Bit 1)
    cfg |= (1 << 1);
    
    // Config Byte schreiben
    {
        if (!ps2_send_cmd(0x60)) {
            gfx_printf("PS2 Controller: Config Byte: send error.\n");
            return 0;
        }   else {
            gfx_printf("PS2 Controller: Config Byte: send ok.\n");
        }
        if (!ps2_write_data(cfg)) {
            gfx_printf("PS2 Controller: Config Byte: write error.\n");
            return 0;
        }   else {
            gfx_printf("PS2 Controller: Config Byte: write ok.\n");
        }
    }
    
    // Maus Defaults
    {
        gfx_printf("PS2 Controller: M defaults.\n");
        if (!ps2_mouse_write(0xF6)) {
            gfx_printf("PS2 Controller: Mouse Defaults: no defaults.\n");
            return 0;
        }   else {
            gfx_printf("PS2 Controller: Mouse Defaults: use deefaults.\n");
        }
        if (!ps2_expect_ack()) {
            gfx_printf("PS2 Controller: Mouse ACK: error.\n");
            return 0;
        }   else {
            gfx_printf("PS2 Controller: Mouse ACK: success.\n");
        }
    }

    // Streaming aktivieren
    {
        if (!ps2_mouse_write(0xF4)) {
            gfx_printf("PS2 Controller: Streaming: error.\n");
            return 0;
        }   else {
            gfx_printf("PS2 Controller: Streaming: ok.\n");
        }
        if (!ps2_expect_ack()) {
            gfx_printf("PS2 Controller: Streaming ACK: error.\n");
            return 0;
        }   else {
            gfx_printf("PS2 Controller: Streaming ACK: success.\n");
        }
    }
    
    return 1;
    
    
    #if 0
    outb(PS2_CMD, cmd);
    
    if (st == 0x55) {
        gfx_printf("PS/2 Controller: ok.\n");
    }   else {
        gfx_printf("PS/2 Controller: fail: 0x%x.\n",st);
        return 0;
    }
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
    #endif
    return 0;
}

extern "C" void mouse_poll(void)
{
    uint8_t b = 0;
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
    uint8_t _status;
    uint8_t b = 0;
    if (!mouse_try_read(&b)) {
        gfx_printf("no data\n");
    }
    
    if (b == 0xFA) {
        gfx_printf("mouse b:      0x%x\n", b);
        gfx_rectFill(100,100,200,300,gfx_rgbColor(0,200,0));
    }
    return;
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
