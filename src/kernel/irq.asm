section .text
BITS 32

global _irq0, _irq1

extern _irq0_handler_c
extern _irq1_handler_c

; IRQ0 – Timer
_irq0:
    cli
    push dword 0       ; dummy error code
    push dword 0x20    ; interrupt number 0x20 - 32

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
    call _irq0_handler_c
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    popa

    add esp, 8

    sti
    iretd

; IRQ1 – Keyboard
_irq1:
    cli
    push dword 0
    push dword 0x21      ; interrupt number 0x21 - 33

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
    call _irq1_handler_c
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    popa

    add esp, 8

    sti
    iretd
