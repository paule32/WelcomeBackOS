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
    . = 0x00040000;
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
    
    /* reject PE-relocations, we have a flat binary */
    /DISCARD/ : {
        *(.reloc)
    }
    
    __end = .;
}
