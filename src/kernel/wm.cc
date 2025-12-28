# include "stdint.h"
# include "proto.h"
# include "kheap.h"
# include "wm.h"

# define DESKTOP
# include "vga.h"

// ====== CONFIG ======
# define WM_MAX_EVENTS   128
# define TITLE_H         24
# define STATUSBAR_H     26
# define BORDER          4
# define RESIZE_GRIP     10

# define WIN_VISIBLE     (1u<<0)
# define WIN_DIRTY       (1u<<1)
# define WIN_MOVING      (1u<<2)
# define WIN_RESIZING    (1u<<3)

// ====== EXTERNAL GFX HOOKS (anpassen!) ======
// Du musst nur diese 3-5 Funktionen an deine gfx_* anpassen.
static inline void gfx_present_to_lfb(const uint32_t *src, int sw, int sh);
static inline void gfx_present_to_lfb_from32(const uint32_t *src, int sw, int sh);
static inline void gfx_text(int x, int y, uint32_t col, const char *s);

// ====== INTERNAL DRAW (software in backbuffer) ======
static int       g_sw, g_sh;
static uint32_t  g_lfb;
static int       g_lfb_pitch;     // pixels
static uint32_t  g_back[ 800 * 600 ]; // backbuffer sw*sh

static window_t *g_z_bottom  = (window_t *)NULL;
static window_t *g_z_top     = (window_t *)NULL;
static window_t *g_focused   = (window_t *)NULL;

static int g_mouse_x = 20, g_mouse_y = 20;
static int g_mouse_buttons = 0;

static int g_drag_off_x      = 0, g_drag_off_y      = 0;
static int g_resize_start_w  = 0, g_resize_start_h  = 0;
static int g_resize_anchor_x = 0, g_resize_anchor_y = 0;

static uint16_t g_cursor_mask[16];
static int g_cursor_w = 16, g_cursor_h = 16;

extern volatile uint32_t lfb_base;   // virtuelle Adresse nach mmio_map
extern USHORT lfb_pitch;             // Bytes pro Scanline

// ====== EVENT QUEUE ======
static event_t g_evq[WM_MAX_EVENTS];
static volatile int g_evq_r = 0;
static volatile int g_evq_w = 0;

static int evq_next(int i) { return (i + 1) % WM_MAX_EVENTS; }
static void wm_compose(void);

void wm_push_event(event_t e) {
    int nw = evq_next(g_evq_w);
    if (nw == g_evq_r) return; // drop if full
    g_evq[g_evq_w] = e;
    g_evq_w = nw;
}

static int wm_pop_event(event_t *out) {
    if (g_evq_r == g_evq_w) return 0;
    *out = g_evq[g_evq_r];
    g_evq_r = evq_next(g_evq_r);
    return 1;
}

// ====== BASIC RECT HELPERS ======
static inline int clampi(int v, int lo, int hi) { return (v < lo) ? lo : (v > hi) ? hi : v; }

static inline int pt_in_rect(int px, int py, int x, int y, int w, int h) {
    return (px >= x && py >= y && px < x + w && py < y + h);
}

static rect_t rect_intersect(rect_t a, rect_t b) {
    int x1 = (a.x > b.x) ? a.x : b.x;
    int y1 = (a.y > b.y) ? a.y : b.y;
    int x2 = ((a.x + a.w) < (b.x + b.w)) ? (a.x + a.w) : (b.x + b.w);
    int y2 = ((a.y + a.h) < (b.y + b.h)) ? (a.y + a.h) : (b.y + b.h);
    rect_t r = { x1, y1, x2 - x1, y2 - y1 };
    if (r.w < 0) r.w = 0;
    if (r.h < 0) r.h = 0;
    return r;
}

static void fill_rect(rect_t r, uint32_t c) {
    rect_t clip = {0,0,g_sw,g_sh};
    r = rect_intersect(r, clip);
    if (r.w <= 0 || r.h <= 0) return;

    for (int y = r.y; y < r.y + r.h; ++y) {
        uint32_t *row = &g_back[y * g_sw + r.x];
        for (int x = 0; x < r.w; ++x) row[x] = c;
    }
}

// ====== WINDOW LIST ======
static void z_detach(window_t *w) {
    if (!w) return;
    if (w->prev) w->prev->next = w->next;
    if (w->next) w->next->prev = w->prev;
    if (g_z_bottom == w) g_z_bottom = w->next;
    if (g_z_top == w) g_z_top = w->prev;
    w->prev = w->next = (window_t *)NULL;
}

static void z_attach_top(window_t *w) {
    if (!w) return;
    w->prev = g_z_top;
    w->next = (window_t *)NULL;
    if (g_z_top) g_z_top->next = w;
    g_z_top = w;
    if (!g_z_bottom) g_z_bottom = w;
}

static void wm_focus(window_t *w) {
    if (g_focused == w) return;
    if (g_focused) g_focused->focused = 0;
    g_focused = w;
    if (g_focused) g_focused->focused = 1;
}

static window_t *wm_window_at(int x, int y) {
    for (window_t *w = g_z_top; w; w = w->prev) {
        if (!(w->flags & WIN_VISIBLE)) continue;
        if (pt_in_rect(x, y, w->x, w->y, w->w, w->h)) return w;
    }
    return (window_t *)NULL;
}

static int hit_title(window_t *w, int mx, int my) {
    return pt_in_rect(mx, my, w->x, w->y, w->w, TITLE_H);
}

static int hit_resize_grip(window_t *w, int mx, int my) {
    int gx = w->x + w->w - RESIZE_GRIP;
    int gy = w->y + w->h - RESIZE_GRIP;
    return pt_in_rect(mx, my, gx, gy, RESIZE_GRIP, RESIZE_GRIP);
}

// ====== WINDOW ALLOC (simple static pool, kein malloc nötig) ======
#define WM_MAX_WINS  16
static window_t g_win_pool[WM_MAX_WINS];
static int g_win_used[WM_MAX_WINS];
static uint32_t g_client_mem[WM_MAX_WINS][320*200]; // NOTFALL: fixed max client buffer

static int next_id = 1;

static void str_copy(char *dst, const char *src, int cap) {
    int i = 0;
    for (; i < cap-1 && src && src[i]; ++i) dst[i] = src[i];
    dst[i] = 0;
}

static void blit_to_lfb(
    const window_t* win,
    uint8_t* lfb_base, uint32_t lfb_pitch,
    int screen_w, int screen_h)
{
    int x = win->x, y = win->y;
    int w = win->w, h = win->h;

    // clipping an screen
    int src_x = 0, src_y = 0;
    if (x < 0) { src_x = -x; w += x; x = 0; }
    if (y < 0) { src_y = -y; h += y; y = 0; }
    if (x + w > screen_w) w = screen_w - x;
    if (y + h > screen_h) h = screen_h - y;
    if (w <= 0 || h <= 0) return;

    for (int row = 0; row < h; ++row) {
        uint8_t* src = (uint8_t*)win->surface.pixels
                     + (uint32_t)(src_y + row) * (uint32_t)win->surface.pitch
                     + (uint32_t)src_x * 2u;

        uint8_t* dst = (uint8_t*)lfb_base
                     + (uint32_t)(y + row) * lfb_pitch
                     + (uint32_t)x * 2u;

        kmemcpy(dst, src, (size_t)w * 2u);
    }
}

window_t *wm_create_window(
    int x,
    int y,
    int w,
    int h,
    const char *title)
{
    int slot = 0;
    window_t *win = &g_win_pool[slot];
    *win = (window_t){0};

    // client surface
    win->client.w = w - 2*BORDER;
    win->client.h = h - STATUSBAR_H - BORDER;
    
    win->surface.w = w;
    win->surface.h = h;
    
    win->surface.pitch  = w * 2;
    win->surface.pixels = (uint16_t*)kmalloc(win->surface.pitch * h);
    
    win->x = x;
    win->y = y;
    win->w = w;
    win->h = h;

    // -------------------------------
    // in den Hintergrund Buffer ...
    // backgriund = border
    // -------------------------------
    gfx_rectFill(
        win->surface.pixels,
        win->surface.pitch,
        0,0,
        win->w, win->h, clRed);
    // clear client
    gfx_rectFill(
        win->surface.pixels,
        win->surface.pitch,
        BORDER,
        BORDER,
        win->client.w, win->client.h, clGreen);
    // title bar
    gfx_rectFill(
        win->surface.pixels,
        win->surface.pitch,
        (2 * BORDER),
        (2 * BORDER),
        win->w   - (2 * BORDER) - 10,
        TITLE_H  + (2 * BORDER), clBlue);
    // client area
    gfx_rectFill(
        win->surface.pixels,
        win->surface.pitch,
        (2 * BORDER),
        TITLE_H + (2 * BORDER) + 10,
        win->w           - (2 * BORDER) - 10,
        win->h - TITLE_H - (2 * BORDER) - 12 - STATUSBAR_H,
        clWhite);
    
    draw_text_Hallo_565(&win->surface, lfb_pitch, 20, 20);
        
    // -------------------------------
    // von Hintergrund Buffer in den
    // Vordergrund zeichnen ...
    // -------------------------------
    blit_to_lfb(win, (uint8_t*)lfb_base, lfb_pitch, lfb_xres, lfb_yres);
//    for (int i = 0; i < win->client.w * win->client.h; ++i)
//        win->client.pixels[i] = 0xFF202020;

    // attach to top + focus
//    z_attach_top(win);
//    wm_focus(win);
    return win;
}

void wm_destroy_window(window_t *w) {
    if (!w) return;
    z_detach(w);
    if (g_focused == w) g_focused = (window_t *)NULL;

    // release pool slot
    for (int i = 0; i < WM_MAX_WINS; ++i) {
        if (&g_win_pool[i] == w) { g_win_used[i] = 0; break; }
    }
}

// ====== DEFAULT PAINT (demo) ======
static void default_paint(window_t *w) {
    /*
    // simple gradient-ish stripes
    for (int y = 0; y < w->client.h; ++y) {
        uint32_t c = 0xFF202020 + ((y & 31) << 8);
        uint32_t *row = &w->client.pixels[y * w->client.pitch];
        for (int x = 0; x < w->client.w; ++x) row[x] = c;
    }
    // you can draw text using your own text routine into client if you have one
    */
}

// ====== DRAW WINDOW FRAME ======
static void draw_window(window_t *w, rect_t clip) {
    // frame rect
    TRect R;
    R.Left   = w->x;
    R.Top    = w->y;
    R.Width  = w->w;
    R.Height = w->h;
    
    //gfx_rectFill(R, clGreen);
    //R   = rect_intersect(R, clip);
    //if (R.w <= 0) return;

    gfx_printf("Wx: %d, Wy: %d\n", w->x, w->y);
    return;

/*
    // client area background behind client surface
    fill_rect((rect_t){w->x+BORDER, w->y+TITLE_H, w->w-2*BORDER, w->h-TITLE_H-BORDER}, 0xFF202020);

    // paint client if dirty
    if ((w->flags & WIN_DIRTY) && w->on_paint) w->on_paint(w);
    if ((w->flags & WIN_DIRTY) && !w->on_paint) default_paint(w);
    w->flags &= ~WIN_DIRTY;

    // blit client
    rect_t client_clip = clip;
    blit_surface(&w->client, w->x+BORDER, w->y+TITLE_H, client_clip);

    // title text (draw directly on backbuffer)
    gfx_text(w->x + 6, w->y + 4, 0xFFFFFFFF, w->title);

    // resize grip visual
    fill_rect((rect_t){w->x+w->w-RESIZE_GRIP, w->y+w->h-RESIZE_GRIP, RESIZE_GRIP, RESIZE_GRIP},
              0xFF606060);*/
}

// ====== CURSOR DRAW (mask bits) ======
static void draw_cursor(void) {
    for (int y = 0; y < g_cursor_h; ++y) {
        uint16_t row = g_cursor_mask[y];
        for (int x = 0; x < g_cursor_w; ++x) {
            if (row & (1u << (15 - x))) { // bit 15 = left
                int px = g_mouse_x + x;
                int py = g_mouse_y + y;
                if ((unsigned)px < (unsigned)g_sw && (unsigned)py < (unsigned)g_sh) {
                    g_back[py * g_sw + px] = 0xFFFFFFFF;
                }
            }
        }
    }
}

// ====== INPUT DISPATCH ======
static void begin_move(window_t *w, int mx, int my) {
    w->flags |= WIN_MOVING;
    g_drag_off_x = mx - w->x;
    g_drag_off_y = my - w->y;
}

static void begin_resize(window_t *w, int mx, int my) {
    w->flags |= WIN_RESIZING;
    g_resize_start_w = w->w;
    g_resize_start_h = w->h;
    g_resize_anchor_x = mx;
    g_resize_anchor_y = my;
}

static void end_drag(window_t *w) {
    if (!w) return;
    w->flags &= ~(WIN_MOVING | WIN_RESIZING);
}

static void handle_mouse_down(int mx, int my, int buttons) {
    (void)buttons;
    window_t *w = wm_window_at(mx, my);
    if (!w) return;

    // bring to top + focus
    z_detach(w);
    z_attach_top(w);
    wm_focus(w);

    if (hit_resize_grip(w, mx, my)) {
        begin_resize(w, mx, my);
    } else if (hit_title(w, mx, my)) {
        begin_move(w, mx, my);
    } else {
        // client click -> later to app
    }
}

static void handle_mouse_move(int mx, int my) {
    window_t *w = g_focused;
    if (!w) return;

    if (w->flags & WIN_MOVING) {
        w->x = clampi(mx - g_drag_off_x, 0, g_sw - 32);
        w->y = clampi(my - g_drag_off_y, 0, g_sh - 32);
    } else if (w->flags & WIN_RESIZING) {
        int dw = mx - g_resize_anchor_x;
        int dh = my - g_resize_anchor_y;
        int nw = clampi(g_resize_start_w + dw, 64, g_sw);
        int nh = clampi(g_resize_start_h + dh, 48, g_sh);

        w->w = nw;
        w->h = nh;

        // resize client surface (simple clamp for fixed buffer demo)
        int cw = nw - 2*BORDER;
        int ch = nh - TITLE_H - BORDER;
        if (cw < 16) cw = 16;
        if (ch < 16) ch = 16;
        if (cw > 320) cw = 320;
        if (ch > 200) ch = 200;

        w->client.w = cw;
        w->client.h = ch;
        w->client.pitch = cw;
        w->flags |= WIN_DIRTY;
    }
}

static void handle_mouse_up(void) {
    if (g_focused) end_drag(g_focused);
}

static void handle_key(int key, int down) {
    if (g_focused && g_focused->on_key) {
        g_focused->on_key(g_focused, key, down);
    }
    // debug: hier könntest du deinen gfx_printf nutzen:
    // gfx_printf("key: %d", key);
}

// ====== COMPOSE ======
static void wm_compose(void) {
    // desktop background
    //gfx_rectFill(0,0,g_sw,g_sh, gfx_rgbColor(0,200,0));

    gfx_printf("gW: %d, gH: &d\n",g_sw,g_sh);
    // draw windows bottom->top
    rect_t clip = {0,0,g_sw,g_sh};
    gfx_printf("utz\n");
    
    window_t *w = (window_t*)kmalloc(sizeof(window_t));
    if (!w) {
        gfx_printf("MEMORY PANIC !!!\n");
        return;
    }
    gfx_printf("MEM OK\n");
    /*w->x = 20;
    w->y = 20;
    w->w = 100;
    w->h = 100;*/
    draw_window(w, clip);
    
    ///for (window_t *w = g_z_bottom; w; w = w->next) {
        //if (!(w->flags & WIN_VISIBLE)) continue;
    //    draw_window(w, clip);
    //}

    // draw cursor last
    //draw_cursor();

    // present
    //gfx_present_to_lfb_from32(g_back, g_sw, g_sh);
}

static inline void lfb_fill565(uint16_t c)
{
    uint16_t *dst = (uint16_t*)(uintptr_t)lfb_base;
    int pitch_px = (int)lfb_pitch / 2;

    for (int y=0; y<600; y++) {
        uint16_t *row = dst + y*pitch_px;
        for (int x=0; x<800; x++) row[x] = c;   // z.B. rot
    }
}
void wm_tick(void) {
    // dispatch queued events
    /*event_t e;
    while (wm_pop_event(&e)) {
        switch (e.type) {
            case EVT_MOUSE_MOVE:
                g_mouse_x = e.mx; g_mouse_y = e.my;
                handle_mouse_move(e.mx, e.my);
                break;
            case EVT_MOUSE_DOWN:
                g_mouse_buttons = e.mb;
                handle_mouse_down(e.mx, e.my, e.mb);
                break;
            case EVT_MOUSE_UP:
                g_mouse_buttons = e.mb;
                handle_mouse_up();
                break;
            case EVT_KEY_DOWN:
                handle_key(e.key, 1);
                break;
            case EVT_KEY_UP:
                handle_key(e.key, 0);
                break;
            default: break;
        }
    }*/

    wm_compose();
}

// ====== INIT ======
void wm_init(
    int screen_w,
    int screen_h,
    uint32_t lfb,
    int lfb_pitch_pixels) {
        
    g_sw = screen_w = lfb_xres;
    g_sh = screen_h = lfb_yres;
    
    g_lfb       = lfb_base;
    g_lfb_pitch = lfb_pitch;

    gfx_printf("sw: %d, sh: %d\n", (size_t)g_sw,(size_t)g_sh);
    /*g_back = (uint32_t*)kmalloc((size_t)g_sw * (size_t)g_sh);
    if (!g_back) {
        gfx_printf("MEMORY PANIC !!!\n");
        return;
    }*/
}

void wm_set_cursor(int w, int h, const uint16_t *mask_bits) {
    if (!mask_bits) return;
    if (w > 16) w = 16;
    if (h > 16) h = 16;
    g_cursor_w = w; g_cursor_h = h;
    for (int i = 0; i < h; ++i) g_cursor_mask[i] = mask_bits[i];
}

// ====== POLLER-INTEGRATION ======
void wm_on_mouse(int mx, int my, int buttons, int relx, int rely, int changed_mask) {
    (void)relx; (void)rely;

    // Move event always
    wm_push_event((event_t){ .type=EVT_MOUSE_MOVE, .mx=mx, .my=my, .mb=buttons });

    // Button transitions
    if (changed_mask & 1) {
        if (buttons & 1) wm_push_event((event_t){ .type=EVT_MOUSE_DOWN, .mx=mx, .my=my, .mb=buttons });
        else             wm_push_event((event_t){ .type=EVT_MOUSE_UP,   .mx=mx, .my=my, .mb=buttons });
    }
}

void wm_on_key(int scancode, int down) {
    wm_push_event((event_t){ .type = down ? EVT_KEY_DOWN : EVT_KEY_UP, .key = scancode });
}

// ====== GFX HOOK IMPLEMENTATION (anpassen) ======
static inline uint16_t xrgb8888_to_rgb565(uint32_t p)
{
    // p: 0x00RRGGBB oder 0xFFRRGGBB (Alpha wird ignoriert)
    uint8_t r = (uint8_t)((p >> 16) & 0xFF);
    uint8_t g = (uint8_t)((p >>  8) & 0xFF);
    uint8_t b = (uint8_t)((p >>  0) & 0xFF);

    // RGB565: R=5bit, G=6bit, B=5bit
    return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

static inline void gfx_present_to_lfb_from32(const uint32_t *src, int sw, int sh)
{
    uint16_t *dst = (uint16_t *)(uintptr_t)lfb_base;
    int dst_pitch_pixels = (int)lfb_pitch / 2;

    for (int y = 0; y < sh; ++y) {
        const uint32_t *s = src + (y * sw);
        uint16_t *d = dst + (y * dst_pitch_pixels);
        for (int x = 0; x < sw; ++x) {
            d[x] = xrgb8888_to_rgb565(s[x]);
        }
    }
}
static inline void gfx_present_to_lfb565(const uint32_t *src, int sw, int sh)
{
    uint16_t *dst = (uint16_t *)(uintptr_t)lfb_base;

    // VBE pitch ist Bytes/Scanline -> in Pixel umrechnen (2 bytes pro Pixel bei 16bpp)
    int dst_pitch_pixels = (int)lfb_pitch / 2;

    for (int y = 0; y < sh; ++y) {
        const uint32_t *s = &src[y * sw];
        uint16_t *d = &dst[y * dst_pitch_pixels];
        for (int x = 0; x < sw; ++x) {
            d[x] = s[x];
        }
    }
}

static inline void gfx_present_to_lfb(
    const uint32_t *src,
    int sw,
    int sh) {
    
    gfx_present_to_lfb_from32(src, sw, sh);
}

static inline void gfx_text(int x, int y, uint32_t col, const char *s) {
    (void)x;
    (void)y;
    (void)col;
    (void)s;
    // HIER: an deine Textausgabe koppeln (z.B. gfx_printf an Position oder font renderer)
    // Minimal: nix tun, dann hast du halt keinen Titeltext.
}
