/* ---------------------------------------------------------------------------
// \file  shell32.ld – A simple freestanding C-Kernel Shell
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
// shell32.ld – 32-Bit Flat Binary, entry point at shell_main
// ---------------------------------------------------------------------------
*/
ENTRY(_shell_main)
SECTIONS
{
    . = 0x00400000;

    .text : {
        LONG(_shell_main);
        __code_start = .;
        *(.text)
    }
    
    .rodata : { *(.rodata*)        }
    .data   : { *(.data*)          }
    .bss    : { *(.bss*) *(COMMON) }

    ___end = .;
}
