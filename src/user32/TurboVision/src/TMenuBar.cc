// ----------------------------------------------------------------------------
// \file  TMenuBar.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"
# include "TurboVision/inc/TObject.h"
# include "TurboVision/inc/TMenuBar.h"

namespace tvision
{
    void rtc_format_hhmmss(char *buf10);

    static inline uint16_t vga_entry(char ch, uint8_t fg, uint8_t bg) {
        uint8_t attr = (uint8_t)((bg << 4) | (fg & 0x0F));
        return (uint16_t)((uint8_t)ch | ((uint16_t)attr << 8));
    }
    
    TMenuBar::TMenuBar(void)
    {
    }

    # define VGA_TEXT_BASE ((volatile uint16_t*)0xB8000)
    # define VGA_COLS 80
    # define VGA_ROWS 25

    void TMenuBar::draw(void)
    {
        char* clockbuf = new char[10];
        rtc_format_hhmmss(clockbuf);
        
        const int y  = 0;  // Zeile 1 (0-basiert 0)
        const int x0 = 0;
        
        const int row_off = y * VGA_COLS;
        
        int x = 0;
        for (; x < VGA_COLS; ++x)
        VGA_TEXT_BASE[row_off + x0 + x] = vga_entry(0xDB, 0x07, 0x00);
        
        for (x = 0; x < 8; ++x)
        VGA_TEXT_BASE[row_off + 71+x] = vga_entry(clockbuf[x], 0, 7);
    }
}
