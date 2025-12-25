section .text
BITS 32

global _irq0, _irq1
extern _irq_handler

; IRQ0 – Timer
_irq0:
    cli
    push dword 0       ; dummy error code
    push dword 32      ; interrupt number 0x20
    jmp irq_common

; IRQ1 – Keyboard
_irq1:
    cli
    push dword 0
    push dword 33      ; interrupt number 0x21
    jmp irq_common

irq_common:
    pusha

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call _irq_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    popa

    add esp, 8

    sti
    iretd
