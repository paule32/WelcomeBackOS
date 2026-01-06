// ----------------------------------------------------------------------------
// \file  shell32.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"
# include "proto.h"

extern "C" void shell_main(void)
{
    // Testmarker, bevor wir springen:
    volatile char* VGA = (volatile char*)0xB8000;
    VGA[0] = 'K'; VGA[1] = 0x0F;
    VGA[2] = 'U'; VGA[3] = 0x0F;
    VGA[4] = 'L'; VGA[5] = 0x0F;
    for(;;);
}
