; int86_blob.asm
global __int86_blob_start
global __int86_blob_end

section .rodata
__int86_blob_start:
    incbin SWITCH_BLOB
__int86_blob_end:
