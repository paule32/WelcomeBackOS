/* ---------------------------------------------------------------------------
// \file  shell.ld – A simple freestanding C-Kernel Shell
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
// shell.ld – 32-Bit Flat Binary, entry point at shell_main
// ---------------------------------------------------------------------------
*/
ENTRY(_shell_main)
SECTIONS
{
    . = 0x00A00000;

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
