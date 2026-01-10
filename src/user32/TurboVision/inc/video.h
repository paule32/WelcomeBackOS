// ----------------------------------------------------------------------------
// \file  TEvent.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __TURBOVISION_VIDEO_H__
#define __TURBOVISION_VIDEO_H__

# include "stdint.h"

//namespace tvision::video {
    volatile uint16_t* VGA = (volatile uint16_t*)0xB8000;
    const int W = 80;
    const int H = 25;

    inline uint16_t cell(char ch, uint8_t attr) {
        return (uint16_t)ch | ((uint16_t)attr << 8);
    }

    inline void put(int x, int y, char ch, uint8_t attr) {
        VGA[y*W + x] = cell(ch, attr);
    }

    inline void fill(int x, int y, int w, int h, char ch, uint8_t attr) {
        for (int yy=0; yy<h; ++yy)
            for (int xx=0; xx<w; ++xx)
                put(x+xx, y+yy, ch, attr);
    }
//}   // namespace: tvision
#endif  // __TURBOVISION_VIDEO_H__
