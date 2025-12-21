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
#define ST_AUX      0x20   // 1 = Daten von Maus (AUX), 0 = Keyboard

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

static int mx = 0;
static int my = 0;

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

volatile uint32_t seen_out = 0;
volatile uint32_t seen_aux = 0;

static inline int ps2_try_read_any(uint8_t *st_out, uint8_t *data_out)
{
    uint8_t st = inb(0x64);
    if (!(st & ST_OUT_FULL)) return 0;
    
    uint8_t d = inb(0x60);
    *st_out = st;
    *data_out = d;
    
    return 1;
}

static void ps2_flush_output(void) {
    for (int i=0;i<64;i++) {
        if (!(inb(0x64) & ST_OUT_FULL)) break;
        (void)inb(0x60);
    }
}

static int ps2_wait_out_full(uint32_t t) {
    while (t--) {
        if (inb(0x64) & ST_OUT_FULL) return 1;
    }
    return 0;
}

static int ps2_wait_in_clear(uint32_t t) {
    while (t--) {
        if ((inb(0x64) & ST_IN_FULL) == 0) return 1;
    }
    return 0;
}

static int ps2_read_mouse_byte(uint8_t *out)
{
    uint8_t st = inb(PS2_STATUS);

    if (!(st & ST_OUT_FULL))
        return 0;               // nichts da -> sofort zurück

    uint8_t d = inb(PS2_DATA);  // MUSS gelesen werden, wenn OUT_FULL gesetzt ist

    if (!(st & ST_AUX))
        return 0;               // war Keyboard/sonstwas -> ignorieren (oder extra behandeln)

    *out = d;
    return 1;
}

static int ps2_expect_mouse_ack(void)
{
    uint8_t b;
    // mehrere Versuche, falls Keyboard dazwischenfunkt
    for (int i = 0; i < 64; i++) {
        if (ps2_read_mouse_byte(&b)) {
            return (b == 0xFA);
        }
    }
    return 0;
}

static int ps2_send_cmd(uint8_t cmd) {
    if (!ps2_wait_in_clear(200000)) return 0;
    outb(0x64, cmd);
    return 1;
}

static int ps2_write_data(uint8_t v) {
    if (!ps2_wait_in_clear(200000)) return 0;
    outb(0x60, v);
    return 1;
}

static int ps2_mouse_write(uint8_t v) {
    if (!ps2_wait_in_clear(200000)) return 0;
    outb(0x64, 0xD4);
    if (!ps2_wait_in_clear(200000)) return 0;
    outb(0x60, v);
    return 1;
}


extern "C" int mouse_install(void)
{
    uint8_t cfg;

    asm volatile("cli");

    ps2_flush_output();

    // AUX (Port 2) aktivieren
    if (!ps2_send_cmd(0xA8)) { asm volatile("sti"); return 0; }

    // Config lesen
    if (!ps2_send_cmd(0x20)) { asm volatile("sti"); return 0; }
    // hier NICHT "ps2_read_data" ohne Filter, wir lesen einfach das nächste Byte
    if (!ps2_wait_out_full(200000)) { asm volatile("sti"); return 0; }
    cfg = inb(0x60);

    // IRQ12 enable
    cfg |= (1 << 1);

    // Config schreiben
    if (!ps2_send_cmd(0x60)) { asm volatile("sti"); return 0; }
    if (!ps2_write_data(cfg)) { asm volatile("sti"); return 0; }

    ps2_flush_output();

    // Defaults optional – wenn du da Probleme hattest: erstmal weglassen
    // ps2_mouse_write(0xF6); ps2_expect_mouse_ack();

    // Data Reporting ON (wichtig!)
    if (!ps2_mouse_write(0xF4)) { asm volatile("sti"); return 0; }
    if (!ps2_expect_mouse_ack()) { asm volatile("sti"); return 0; }

    asm volatile("sti");
    
    return 1;
}

extern "C" void mouse_poll(void)
{
    uint8_t st, b;

    if (!ps2_try_read_any(&st, &b))
        return;

    seen_out++;
    if (st & ST_AUX) seen_aux++;
gfx_drawCircleFill(mx,my,40,gfx_rgbColor(255,0,255));
    // nur wenn es wirklich von der Maus ist:
    if (!(st & ST_AUX))
        return;

    if (mouse_cycle == 0 && !(b & 0x08)) return;
    mouse_byte[mouse_cycle++] = b;
    if (mouse_cycle < 3) return;
    mouse_cycle = 0;

    int8_t dx = (int8_t)mouse_byte[1];
    int8_t dy = (int8_t)mouse_byte[2];

    mx += dx;
    my -= dy;

    if (mx < 0) mx = 0;
    if (my < 0) my = 0;
    if (mx >= lfb_xres) mx = lfb_xres - 1;
    if (my >= lfb_yres) my = lfb_yres - 1;

    cursor_update(mx, my);
}
