; int86_blob.asm
global __int86_blob_start
global __int86_blob_end

section .rodata
__int86_blob_start:
    incbin "../bin/int86_blob.bin"
__int86_blob_end:
