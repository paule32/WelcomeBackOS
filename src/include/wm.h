#ifndef __WM_H__
#define __WM_H__

# include "stdint.h"
# include "proto.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct { int x, y, w, h; } rect_t;

typedef struct {
    int w, h;
    int pitch;          // in pixels
    uint32_t *pixels;   // XRGB/ARGB
} surface_t;

typedef enum {
    EVT_NONE = 0,
    EVT_MOUSE_MOVE,
    EVT_MOUSE_DOWN,
    EVT_MOUSE_UP,
    EVT_KEY_DOWN,
    EVT_KEY_UP
} evt_type_t;

typedef struct {
    evt_type_t type;
    int mx, my;         // mouse position
    int mb;             // button bitmask (1=left,2=right,4=middle)
    int key;            // scancode or keycode
} event_t;

typedef struct window window_t;

typedef void (*win_paint_fn)(window_t *w);
typedef void (*win_key_fn)(window_t *w, int key, int down);

struct window {
    int id;
    int x, y, w, h;
    int focused;
    uint32_t flags;

    surface_t client;       // client surface (w x (h-title))
    char title[32];

    win_paint_fn on_paint;
    win_key_fn   on_key;

    window_t *prev, *next;  // z-order list
};

void wm_init(int screen_w, int screen_h, uint32_t lfb, int lfb_pitch_pixels);
void wm_set_cursor(int w, int h, const uint16_t *mask_bits); // optional
window_t *wm_create_window(int x, int y, int w, int h, const char *title);
void wm_destroy_window(window_t *w);

void wm_push_event(event_t e);
void wm_tick(void);     // dispatch events + compose + present

// Optional helpers to integrate your pollers
void wm_on_mouse(int mx, int my, int buttons, int relx, int rely, int changed_buttons_mask);
void wm_on_key(int scancode, int down);

#ifdef __cplusplus
};
#endif

#endif  // __WM_H__