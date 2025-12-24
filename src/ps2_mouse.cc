// ps2_mouse.c

# include "stdint.h"
# include "proto.h"

# define DESKTOP 1
# include "vga.h"

# include "wm.h"

// Ports
#define PS2_DATA      0x60
#define PS2_STATUS    0x64
#define PS2_CMD       0x64

// Status bits
#define ST_OUT_FULL   0x01
#define ST_IN_FULL    0x02
#define ST_AUX        0x20   // 1 = Daten von Maus (AUX), 0 = Keyboard

#define MOUSE_LEFT    0x01
#define MOUSE_RIGHT   0x02
#define MOUSE_MIDDLE  0x04

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
    0b1110111000000000,
    0b1100011100000000,
    0b1000001110000000,
    0b0000000111000000,
    0b0000000011000000,
};

signed char mouse_cycle = 0;
signed char mouse_byte[ 3 ];

uint16_t mouse_x = 0;
uint16_t mouse_y = 0;

static int mx = 0;
static int my = 0;

static uint8_t mouse_buttons      = 0;
static uint8_t mouse_buttons_last = 0;

typedef struct {
    int old_x, old_y;
    int saved_x, saved_y;    // tatsächlich gesicherte Ecke (geclippt)
    int saved_w, saved_h;    // tatsächlich gesicherte Größe (geclippt)
    bool has_saved;
    
    // Hintergrundpuffer max 16x16 Pixel
    uint32_t bg32[CUR_W * CUR_H];
    uint16_t bg16[CUR_W * CUR_H];
}   cursor_overlay_t;

static cursor_overlay_t g_cur = {
    .old_x = -9999, .old_y = -9999,
    .has_saved = false
};

static inline uint16_t fb_get16(int x, int y) {
    return gfx_getPixel(x,y);
}
static inline void fb_put16(int x, int y, uint16_t c) {
    gfx_putPixel(x,y,c);
}

static inline int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// Berechnet geclippten Cursor-Rechteckbereich auf dem Screen
static void cursor_clip_rect(
    int   x, int   y,
    int *sx, int *sy,
    int *sw, int *sh) {
        
    int x0 = x;
    int y0 = y;
    int x1 = x + CUR_W;
    int y1 = y + CUR_H;

    // Screen clip
    int cx0 = clampi(x0, 0, (int)lfb_xres);
    int cy0 = clampi(y0, 0, (int)lfb_yres);
    int cx1 = clampi(x1, 0, (int)lfb_xres);
    int cy1 = clampi(y1, 0, (int)lfb_yres);

    *sx = cx0;
    *sy = cy0;
    *sw = cx1 - cx0;
    *sh = cy1 - cy0;
}
static void cursor_save_bg(int x, int y)
{
    int sx, sy, sw, sh;
    cursor_clip_rect(x, y, &sx, &sy, &sw, &sh);

    g_cur.saved_x = sx;
    g_cur.saved_y = sy;
    g_cur.saved_w = sw;
    g_cur.saved_h = sh;

    if (sw <= 0 || sh <= 0) {
        g_cur.has_saved = false;
        return;
    }

    if (lfb_bpp == 16) {
        for (int yy = 0; yy < sh; yy++) {
            for (int xx = 0; xx < sw; xx++) {
                g_cur.bg16[yy * CUR_W + xx] = gfx_getPixel(sx + xx, sy + yy);
            }
        }
    }

    g_cur.has_saved = true;
}
static void cursor_restore_bg(void)
{
    if (!g_cur.has_saved) return;

    int sx = g_cur.saved_x, sy = g_cur.saved_y;
    int sw = g_cur.saved_w, sh = g_cur.saved_h;

    if (lfb_bpp == 16) {
        for (int yy = 0; yy < sh; yy++) {
            for (int xx = 0; xx < sw; xx++) {
                fb_put16(sx + xx, sy + yy, g_cur.bg16[yy * CUR_W + xx]);
            }
        }
    }

    g_cur.has_saved = false;
}

static void cursor_draw(int x, int y)
{
    int sx, sy, sw, sh;
    cursor_clip_rect(x, y, &sx, &sy, &sw, &sh);
    if (sw <= 0 || sh <= 0) return;

    // Offset, falls Cursor links/oben aus dem Screen raussteht
    int offx = sx - x;
    int offy = sy - y;

    if (lfb_bpp == 16) {
        const uint16_t col = gfx_rgbColor(255,255,255);
        for (int yy = 0; yy < sh; yy++) {
            uint16_t rowmask = cursor_mask[offy + yy];
            for (int xx = 0; xx < sw; xx++) {
                int bit = 15 - (offx + xx);
                if (bit >= 0 && bit < 16) {
                    if (rowmask & (1u << bit)) {
                        fb_put16(sx + xx, sy + yy, col);
                    }
                }
            }
        }
    }
}

void cursor_overlay_move(int new_x, int new_y)
{
    // 1) alten Bereich zurück
    cursor_restore_bg();

    // 2) neuen Bereich sichern
    cursor_save_bg(new_x, new_y);

    // 3) Cursor drüber zeichnen
    cursor_draw(new_x, new_y);

    g_cur.old_x = new_x;
    g_cur.old_y = new_y;
}

void cursor_update(int x, int y)
{
    cursor_overlay_move(mx, my);
}

volatile uint32_t seen_out = 0;
volatile uint32_t seen_aux = 0;

static inline int ps2_try_read_any(
    uint8_t *st_out,
    uint8_t *data_out) {
    uint8_t st = inb(0x64);
    
    if (!(st & ST_OUT_FULL)) return 0;
    
    uint8_t d = inb(0x60);
    *st_out = st;
    *data_out = d;
    
    return 1;
}

static bool mouse_poll_packet(uint8_t packet[3])
{
    uint8_t st;
    for (int i = 0; i < 3; i++) {
        if (!ps2_try_read_any(&st, &packet[i]))
            return false;
    }
    return true;
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

// --- Keyboard: Set 1 Scancode (raw) ---
static void kbd_process_byte(uint8_t sc)
{
    // Optional: nur Make-Codes anzeigen (Key-Down). Break hat Bit7 gesetzt.
    if (sc & 0x80) return;

    gfx_printf("key: %d  ", (int)sc);
}

static inline bool mouse_in_rect_xy(int px, int py, int x, int y, int w, int h) {
    return(
    px >= x    &&
    py >= y    &&
    
    px < x + w &&
    py < y + h );
}

static void mouse_handle_clicks(int px, int py, uint8_t buttons)
{
    bool left_now  = (buttons & MOUSE_LEFT) != 0;
    bool left_prev = (mouse_buttons_last & MOUSE_LEFT) != 0;

    if (left_now && !left_prev) {
        if (mouse_in_rect_xy(px, py, 100, 100, 200, 40))
            gfx_printf("Button bei (100,100) geklickt!\n");

        if (mouse_in_rect_xy(px, py, 60, 60, 320, 200))
            gfx_printf("Fenster angeklickt!\n");
    }

    mouse_buttons_last = buttons;
}

static void mouse_process_byte(uint8_t b)
{
    // Sync-Bit (Bit3) muss beim ersten Byte gesetzt sein
    if (mouse_cycle == 0 && !(b & 0x08)) return;

    mouse_byte[mouse_cycle++] = (signed char)b;
    if (mouse_cycle < 3) return;
    mouse_cycle = 0;

    uint8_t b0 = (uint8_t)mouse_byte[0];
    int8_t  dx = (int8_t )mouse_byte[1];
    int8_t  dy = (int8_t )mouse_byte[2];

    mouse_buttons = b0 & (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE);

    mx += dx;
    my -= dy; // Y invertiert

    mx = clampi(mx, 0, (int)lfb_xres - 1);
    my = clampi(my, 0, (int)lfb_yres - 1);

    cursor_overlay_move(mx, my);

    // Klicks auf Basis der echten Cursor-Pos
    mouse_handle_clicks(mx, my, mouse_buttons);
}

extern "C" int mouse_install(void)
{
    uint8_t cfg;

    asm volatile("cli");

    ps2_flush_output();

    // AUX (Port 2) aktivieren
    if (!ps2_send_cmd(0xA8)) { asm volatile("sti"); return 0; }
    gfx_printf("PS2 Controller: Port 2 active.\n");

    // Config lesen
    if (!ps2_send_cmd(0x20)) { asm volatile("sti"); return 0; }
    gfx_printf("PS2 Controller: Config read: ok.\n");
    
    // hier NICHT "ps2_read_data" ohne Filter, wir lesen einfach das nächste Byte
    if (!ps2_wait_out_full(200000)) { asm volatile("sti"); return 0; }
    cfg = inb(0x60);

    // IRQ12 enable
    cfg |= (1 << 1);

    // Config schreiben
    if (!ps2_send_cmd(0x60)) { asm volatile("sti"); return 0; }
    if (!ps2_write_data(cfg)) { asm volatile("sti"); return 0; }
    gfx_printf("PS2 Controller: Config write: ok.\n");

    ps2_flush_output();

    // Defaults optional – wenn du da Probleme hattest: erstmal weglassen
    // ps2_mouse_write(0xF6); ps2_expect_mouse_ack();

    // Data Reporting ON (wichtig!)
    if (!ps2_mouse_write(0xF4)) { asm volatile("sti"); return 0; }
    gfx_printf("PS2 Controller: Reporting: ok.\n");
    
    if (!ps2_expect_mouse_ack()) { asm volatile("sti"); return 0; }
    gfx_printf("PS2 Controller: ACK: ok.\n");

    //asm volatile("sti");
    return 1;
}

extern "C" void mouse_poll(void)
{
    //wm_on_mouse(mx, my, buttons, relx, rely, changed_mask);
    
    uint8_t st, d;
    if (!ps2_try_read_any(&st, &d))
    return;

    if (st & ST_AUX) { mouse_process_byte(d); }
    else             {   kbd_process_byte(d); }
}
