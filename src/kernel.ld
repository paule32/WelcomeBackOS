/* ---------------------------------------------------------------------------
// \file kernel.ld – A simple freestanding C-Kernel
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
// kernel.ld – 32-Bit Flat Binary, entry point at kmain
// ---------------------------------------------------------------------------
*/
/*OUTPUT_FORMAT("binary")*/
ENTRY(KernelStart)
SECTIONS
{ 
    . = 0x000400000;
    .text : {
        LONG(_kmain);
        __code_start = .;
        *(EXCLUDE_FILE(
            ../obj/shared_pages.o
            ../obj/data.o
            )
            .text .text.*
        )
        
        ../obj/shared_pages.o(.text)
        ../obj/data.o(.text)
    }
    
    . = ALIGN(0x1000);
    .rodata : {
        __rodata_start = .;
        *(.rodata) *(.rdata)
    }
    
    . = ALIGN(0x1000);
    .data : {
        __data_start = .;
        *(.data)
    }
    
    . = ALIGN(0x1000);
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss) *(COMMON)
        __bss_end = .;
    }
    
    __end = .;
    
    /* reject PE-relocations, we have a flat binary */
    /DISCARD/ : {
        *(.reloc)
    }
}
