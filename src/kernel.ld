/* ---------------------------------------------------------------------------
// \file kernel.ld – A simple freestanding C-Kernel
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
// kernel.ld – 32-Bit Flat Binary, entry point at kmain
// ---------------------------------------------------------------------------
*/
/*OUTPUT_FORMAT("binary")*/
ENTRY(_kmain)
SECTIONS
{ 
    . = 0x00010000;
    .text : {
        LONG(_kmain);
        __code_start = .;
        *(.text)
    }
    
    .rodata : {
        __rodata_start = .;
        *(.rodata) *(.rdata)
    }
    
    .data : {
        __data_start = .;
        *(.data)
    }
    
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss) *(COMMON)
        __bss_end = .;
    }
    
    ___end = .;
    
    /* reject PE-relocations, we have a flat binary */
    /DISCARD/ : {
        *(.reloc)
    }
}
