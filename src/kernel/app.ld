/* app.ld - Linker Script für User-ELF (i386), feste Basis 0x00400000 */
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    /* feste User-Basis */
    . = 0x00400000;

    /* Code */
    .text : ALIGN(4096) {
        *(.text .text.*)
    }

    /* Read-only Daten */
    .rodata : ALIGN(4096) {
        *(.rodata .rodata.*)
    }

    /* Daten */
    .data : ALIGN(4096) {
        *(.data .data.*)
    }

    /* BSS (wird vom Loader genullt) */
    .bss : ALIGN(4096) {
        *(COMMON)
        *(.bss .bss.*)
    }

    /* Optional: Debug/Notes rauswerfen */
    /DISCARD/ : {
        *(.comment*)
        *(.note*)
        *(.eh_frame*)
    }
}