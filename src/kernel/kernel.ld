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
        *(.text .text.*)
    } 
    
    .rodata : {
        __rodata_start = .;
        *(.rodata .rodata.* .rdata .rdata.*)
    }
    
    .init_array : ALIGN(4) {
        __init_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
    }
    
    .data : {
        __data_start = .;
        *(.data .data.*)
    }
    
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss .bss.*) *(COMMON)
        __bss_end = .;
    }
    
    ___end = .;
    
    /* reject PE-relocations, we have a flat binary */
    /DISCARD/ : {
        *(.eh_frame) *(.eh_frame_hdr)
        *(.gcc_except_table)
        *(.reloc)
        *(.comment) *(.note*) 
    }
}
