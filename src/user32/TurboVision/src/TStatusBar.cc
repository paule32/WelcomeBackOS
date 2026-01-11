// ----------------------------------------------------------------------------
// \file  TStatusBar.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"

# include "TurboVision/inc/TObject.h"
# include "TurboVision/inc/TStatusBar.h"

namespace tvision
{
    static inline uint16_t vga_entry(char ch, uint8_t fg, uint8_t bg) {
        uint8_t attr = (uint8_t)((bg << 4) | (fg & 0x0F));
        return (uint16_t)((uint8_t)ch | ((uint16_t)attr << 8));
    }
    
    # define VGA_TEXT_BASE ((volatile uint16_t*)0xB8000)
    # define VGA_COLS 80
    # define VGA_ROWS 25
    
    TStatusBar::TStatusBar(void)
    {
    }
    
    void TStatusBar::draw(void)
    {
        const int y  = VGA_ROWS - 1;  // Zeile 25 (0-basiert 24)
        const int x0 = 0;
        
        const int row_off = y * VGA_COLS;
        
        int x = 0;
        for (; x < VGA_COLS; ++x)
        VGA_TEXT_BASE[row_off + x0 + x] = vga_entry(0xDB, 0x07, 0x00);
    }
}
