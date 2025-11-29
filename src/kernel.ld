/* ---------------------------------------------------------------------------
// \file kernel.c – A simple freestanding C-Kernel
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
// kernel.ld – 32-Bit Flat Binary, entry point at kmain
// ---------------------------------------------------------------------------
*/
OUTPUT_FORMAT("binary")
ENTRY(KernelStart)
SECTIONS
{ 
    . = 0x00040000;
    .text: {
        __code_start = .;
        *(EXCLUDE_FILE(
            ../obj/shared_pages.o
            ../obj/data.o
        ).text)
    } 
    
    . = 0x00045000;
    .text1  : {
        ../obj/shared_pages.o(.text)
    } 
    
    . = 0x00046000;
    .text2  : { data.o(.text)                            }
    .data   : { __data_start = .;   *(.data)             } 	
    .rodata : { __rodata_start = .; *(.rodata) *(.rdata) }
    .bss    : { __bss_start = .;    *(.bss) *(COMMON)    }
    
    __end = .;
}
