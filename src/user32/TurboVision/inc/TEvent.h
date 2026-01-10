// ----------------------------------------------------------------------------
// \file  TEvent.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __TURBOVISION_TEVENT_H__
#define __TURBOVISION_TEVENT_H__

# include "stdint.h"

typedef enum {
    evNone      = 0,
    evKeyDown   = 1,
    evKeyUp     = 2,
    evCommand   = 3,
    evBroadcast = 4,
    evIdle      = 5
}   event_what_t;

typedef struct {
    uint8_t shift, ctrl, alt, caps;
}   kbd_mod_t;

typedef struct {
    uint8_t scancode;   // 0..127 (ohne break bit)
    uint8_t released;   // 0 press, 1 release
    uint8_t e0;         // 1 bei E0-prefix
    uint8_t ascii;      // 0 wenn keins
    kbd_mod_t mod;      // Snapshot
}   key_event_t;

typedef struct {
    // Turbo-Vision style: "keyCode" trägt entweder ASCII oder ein Special-Key
    uint16_t keyCode;      // z.B. 'a' oder kbF1/kbUp/...
    uint8_t  scanCode;     // raw scancode (0..127)
    uint8_t  charCode;     // ascii (0..255)
    kbd_mod_t mod;
}   key_down_t;

typedef struct TView TView;   // falls TEvent irgendwo TView referenziert
typedef struct TEvent {
    uint16_t what;
    union {
        key_down_t keyDown;
        struct {
            uint16_t command;
            uint32_t param;     // optional: pointer/handle/int
        } message;
        struct {
            int x;
            int y;
            uint16_t buttons;
        }   mouse;
    }   data;
}   TEvent;

#endif  // __TURBOVISION_TEVENT_H__
