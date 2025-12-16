BITS 32
section .text

global __gdt_flush
__gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]
    ; Segmente neu laden
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush_done
.flush_done:
    ret

global __tss_flush
__tss_flush:
    mov ax, 0x28      ; TSS-Selector: Index 5, RPL=0 -> 5*8 = 0x28
    ltr ax
    ret
