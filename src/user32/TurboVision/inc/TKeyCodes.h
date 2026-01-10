// ----------------------------------------------------------------------------
// \file  TKeyCodes.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __TURBOVISION_TKEYCODES_H__
#define __TURBOVISION_TKEYCODES_H__

enum {
    kbEsc   = 27,
    kbEnter = '\n',
    kbTab   = '\t',
    kbBack  = '\b',

    // Special keys (oberhalb ASCII-Bereich)
    kbUp    = 0x100,
    kbDown  = 0x101,
    kbLeft  = 0x102,
    kbRight = 0x103,
    kbHome  = 0x104,
    kbEnd   = 0x105,
    kbPgUp  = 0x106,
    kbPgDn  = 0x107,
    kbIns   = 0x108,
    kbDel   = 0x109,

    kbF1    = 0x120,
    kbF2    = 0x121,
    kbF3    = 0x122,
    kbF4    = 0x123,
    kbF5    = 0x124,
    kbF6    = 0x125,
    kbF7    = 0x126,
    kbF8    = 0x127,
    kbF9    = 0x128,
    kbF10   = 0x129,
    kbF11   = 0x12A,
    kbF12   = 0x12B
};

#endif  // __TURBOVISION_TKEYCODES_H__
