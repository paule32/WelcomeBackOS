// ----------------------------------------------------------------------------
// \file  shell32_app.c
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"

# include "TurboVision/inc/TObject.h"
# include "TurboVision/inc/TApplication.h"
# include "TurboVision/inc/TPoint.h"


#ifdef __00__
# include "TurboVision/inc/TView.h"
# include "TurboVision/inc/TEvent.h"
# include "TurboVision/inc/TCommandIDs.h"
# include "TurboVision/inc/TKeyCodes.h"

# define VGA_MEM ((volatile uint16_t*)0xB8000)
# define VGA_W   80
# define VGA_H   25

static uint8_t g_attr = 0x1F; // white on blue (Beispiel)

static void vga_putxy(int x, int y, char ch, uint8_t attr)
{
    if ((unsigned)x >= VGA_W || (unsigned)y >= VGA_H) return;
    VGA_MEM[y * VGA_W + x] = (uint16_t)ch | ((uint16_t)attr << 8);
}
static void vga_fill(int x, int y, int w, int h, char ch, uint8_t attr)
{
    for (int yy = 0; yy < h; yy++)
        for (int xx = 0; xx < w; xx++)
            vga_putxy(x + xx, y + yy, ch, attr);
}
static void vga_printxy(int x, int y, const char *s, uint8_t attr)
{
    while (*s && x < VGA_W) {
        vga_putxy(x++, y, *s++, attr);
    }
}
static void vga_clear(uint8_t attr)
{
    extern void clear_screen2();
    clear_screen2();
    //vga_fill(0, 0, VGA_W, VGA_H, 0xB0, attr);
}

bool keyq_pop(key_event_t *out)
{
    /*
    uint32_t flags = irq_save();

    uint32_t tail = g_keyq.tail;
    if (tail == g_keyq.head) {
        irq_restore(flags);
        return false; // leer
    }

    *out = g_keyq.buf[tail];
    g_keyq.tail = (tail + 1u) & KEYQ_MASK;

    irq_restore(flags);*/
    return true;
}

// Minimal: scancode->special mapping (Set 1, inkl. E0 für Pfeile)
static uint16_t scancode_to_special(uint8_t sc, bool e0)
{
    if (!e0) {
        switch (sc) {
            case 0x3B: return kbF1;
            case 0x3C: return kbF2;
            case 0x3D: return kbF3;
            case 0x3E: return kbF4;
            case 0x3F: return kbF5;
            case 0x40: return kbF6;
            case 0x41: return kbF7;
            case 0x42: return kbF8;
            case 0x43: return kbF9;
            case 0x44: return kbF10;
            // F11/F12 sind je nach Layout/Set anders; kannst du später ergänzen
        }
    } else {
        switch (sc) {
            case 0x48: return kbUp;     // E0 48
            case 0x50: return kbDown;   // E0 50
            case 0x4B: return kbLeft;   // E0 4B
            case 0x4D: return kbRight;  // E0 4D
            case 0x47: return kbHome;   // E0 47
            case 0x4F: return kbEnd;    // E0 4F
            case 0x49: return kbPgUp;   // E0 49
            case 0x51: return kbPgDn;   // E0 51
            case 0x52: return kbIns;    // E0 52
            case 0x53: return kbDel;    // E0 53
        }
    }
    return 0;
}
static void getEvent(TEvent *ev)
{
    ev->what = evNone;

    key_event_t k;
    if (!keyq_pop(&k)) {
        ev->what = evIdle;
        return;
    }

    ev->what = k.released ? evKeyUp : evKeyDown;

    ev->data.keyDown.scanCode = k.scancode;
    ev->data.keyDown.charCode = k.ascii;
    ev->data.keyDown.mod      = k.mod;

    uint16_t special = scancode_to_special(k.scancode, k.e0);

    if (!k.released) {
        // KeyDown: keyCode = special oder ascii
        if (special) ev->data.keyDown.keyCode = special;
        else         ev->data.keyDown.keyCode = (uint16_t)k.ascii;
    } else {
        // KeyUp: optional keyCode füllen (nicht zwingend)
        ev->data.keyDown.keyCode = special ? special : (uint16_t)k.ascii;
    }
}
static void group_add(TGroup *g, TView *v)
{
    if (g->count < 8) {
        g->children[g->count++] = v;
        v->owner = (TView*)g;
    }
}
static void group_focus(TGroup *g, TView *v)
{
    if (!v || !v->selectable) return;
    if (g->current) g->current->focused = false;
    g->current = v;
    g->current->focused = true;
}
static void group_focus_next(TGroup *g)
{
    if (g->count == 0) return;
    uint32_t start = 0;

    if (g->current) {
        for (uint32_t i=0;i<g->count;i++) if (g->children[i]==g->current) { start=i; break; }
    }
    for (uint32_t step=1; step<=g->count; step++) {
        uint32_t i = (start + step) % g->count;
        if (g->children[i]->selectable) { group_focus(g, g->children[i]); return; }
    }
}
static void group_focus_prev(TGroup *g)
{
    if (g->count == 0) return;
    uint32_t start = 0;

    if (g->current) {
        for (uint32_t i=0;i<g->count;i++) if (g->children[i]==g->current) { start=i; break; }
    }
    for (uint32_t step=1; step<=g->count; step++) {
        uint32_t i = (start + g->count - step) % g->count;
        if (g->children[i]->selectable) { group_focus(g, g->children[i]); return; }
    }
}

static bool group_handleEvent(TView *self_, TEvent *ev)
{
    TGroup *g = (TGroup*)self_;

    // Fokuswechsel als Command
    if (ev->what == evCommand) {
        if (ev->data.message.command == cmNext) { group_focus_next(g); return true; }
        if (ev->data.message.command == cmPrev) { group_focus_prev(g); return true; }
    }

    // zuerst focused child
    if (g->current && g->current->handleEvent) {
        if (g->current->handleEvent(g->current, ev)) return true;
    }

    return false;
}
static void group_draw(TView *self_)
{
    TGroup *g = (TGroup*)self_;
    for (uint32_t i=0;i<g->count;i++)
        if (g->children[i]->draw) g->children[i]->draw(g->children[i]);
}

static void draw_frame(int x, int y, int w, int h, uint8_t attr)
{
    // simple ASCII frame
    vga_putxy(x, y, '+', attr);
    vga_putxy(x+w-1, y, '+', attr);
    vga_putxy(x, y+h-1, '+', attr);
    vga_putxy(x+w-1, y+h-1, '+', attr);

    for (int i=1;i<w-1;i++) { vga_putxy(x+i, y, '-', attr); vga_putxy(x+i, y+h-1, '-', attr); }
    for (int i=1;i<h-1;i++) { vga_putxy(x, y+i, '|', attr); vga_putxy(x+w-1, y+i, '|', attr); }
}

static void edit_draw(TView *self_)
{
    TEditView *e = (TEditView*)self_;
    uint8_t attr = e->base.focused ? 0x1E : 0x1F; // gelb/blue vs white/blue

    draw_frame(e->base.x, e->base.y, e->base.w, e->base.h, attr);

    // title
    if (e->title) vga_printxy(e->base.x+2, e->base.y, e->title, attr);

    // inner area
    int ix = e->base.x + 1;
    int iy = e->base.y + 1;
    int iw = e->base.w - 2;
    int ih = e->base.h - 2;

    vga_fill(ix, iy, iw, ih, 0xB0, attr);

    // print buffer on first line (demo)
    int max = (iw < 255) ? iw : 255;
    for (int i=0; i<max && i<(int)e->len; i++)
        vga_putxy(ix + i, iy, e->buf[i], attr);

    // cursor (demo: invert cell)
    if (e->base.focused) {
        int cx = ix + (int)e->cursor;
        if (cx < ix+iw) {
            // invert: black on yellow
            vga_putxy(cx, iy, (e->cursor < e->len) ? e->buf[e->cursor] : ' ', 0xE0);
        }
    }
}

static bool edit_handleEvent(TView *self_, TEvent *ev)
{
    TEditView *e = (TEditView*)self_;

    if (ev->what == evKeyDown) {
        uint16_t kc = ev->data.keyDown.keyCode;
        uint8_t ch  = ev->data.keyDown.charCode;

        if (kc == kbLeft)  { if (e->cursor) e->cursor--; return true; }
        if (kc == kbRight) { if (e->cursor < e->len) e->cursor++; return true; }
        if (kc == kbBack) {
            if (e->cursor && e->len) {
                // delete before cursor
                for (uint32_t i=e->cursor-1; i<e->len-1; i++) e->buf[i] = e->buf[i+1];
                e->len--; e->cursor--;
            }
            return true;
        }

        if (ch >= 32 && ch < 127) {
            if (e->len < sizeof(e->buf)-1) {
                // insert at cursor
                for (uint32_t i=e->len; i>e->cursor; i--) e->buf[i] = e->buf[i-1];
                e->buf[e->cursor] = (char)ch;
                e->len++; e->cursor++;
                e->buf[e->len] = 0;
            }
            return true;
        }
    }

    if (ev->what == evCommand) {
        // nur Demo: schreibe Command-Text in Buffer
        const char *msg = 0;
        switch (ev->data.message.command) {
            case cmCopy:  msg = "[COPY]"; break;
            case cmPaste: msg = "[PASTE]"; break;
            case cmCut:   msg = "[CUT]"; break;
            default: break;
        }
        if (msg) {
            while (*msg) {
                if (e->len < sizeof(e->buf)-1) { e->buf[e->len++] = *msg++; e->buf[e->len]=0; }
                else break;
            }
            e->cursor = e->len;
            return true;
        }
    }

    return false;
}

static void draw_status(const char *text)
{
    // Statusline unten
    vga_fill(0, VGA_H-1, VGA_W, 1, ' ', 0x70); // black on light gray
    vga_printxy(1, VGA_H-1, text, 0x70);
}

static void app_init____(TApplication *app)
{
    // Desktop group init
    app->desktop.base.draw = group_draw;
    app->desktop.base.handleEvent = group_handleEvent;
    app->desktop.count = 0;
    app->desktop.current = 0;

    // Left edit view
    app->left.base.x=1; app->left.base.y=1; app->left.base.w=38; app->left.base.h=10;
    app->left.base.selectable = true;
    app->left.base.draw = edit_draw;
    app->left.base.handleEvent = edit_handleEvent;
    app->left.title = " Left ";
    app->left.len=0; app->left.cursor=0; app->left.buf[0]=0;

    // Right edit view
    app->right.base.x=41; app->right.base.y=1; app->right.base.w=38; app->right.base.h=10;
    app->right.base.selectable = true;
    app->right.base.draw = edit_draw;
    app->right.base.handleEvent = edit_handleEvent;
    app->right.title = " Right ";
    app->right.len=0; app->right.cursor=0; app->right.buf[0]=0;

    group_add(&app->desktop, (TView*)&app->left);
    group_add(&app->desktop, (TView*)&app->right);

    group_focus(&app->desktop, (TView*)&app->left);

    vga_clear(0x1E);
    draw_status("TAB wechseln | ESC quit | F1 help | Ctrl+C/V/X");
    app->desktop.base.draw((TView*)&app->desktop);
}

static void app_handle(TApplication *app, TEvent *ev)
{
    // globale commands
    if (ev->what == evCommand) {
        switch (ev->data.message.command) {
            case cmQuit:
                draw_status("Quit requested (ESC).");
                app->running = false;
                return;
            case cmHelp:
                draw_status("HELP: Tippe Text. TAB wechselt Fokus. Ctrl+C/V/X Demo.");
                // nicht consume? TurboVision konsumiert meist:
                break;
            default: break;
        }
    }

    // an desktop dispatchen
    if (app->desktop.base.handleEvent)
        app->desktop.base.handleEvent((TView*)&app->desktop, ev);

    // nach jedem Event neu zeichnen (Demo-Style)
    draw_status("TAB wechseln | ESC quit | F1 help | Ctrl+C/V/X");
    app->desktop.base.draw((TView*)&app->desktop);
}
#endif

extern "C" void printformat(const char* fmt, ...); 
extern "C" void app_run_demo(void)
{
    using namespace tvision;
    TPoint pt1, pt2, pt3;
    
    pt1.x = 2;
    pt2.x = 3;
    pt3   = pt1 ^ pt2;
    printformat("--> %d\n", pt3.x);
    auto *app = new TApplication();
    app->Init();
    app->run ();
    
    //delete app;
/*
    app.running = true;
    while (app.running) {
        TEvent ev;
        getEvent(&ev);

        if (ev.what == evIdle) {
            // optional idle: cursor blink, tick, etc.
            continue;
        }

        app_handle(&app, &ev);
    }*/

    // nach Quit: in eine harmlose Loop (oder zurück zu deinem Kernel-Menü)
    for (;;) { __asm__ volatile ("hlt"); }
}
