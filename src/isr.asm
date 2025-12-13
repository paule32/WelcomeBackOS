; isr_stubs.asm
section .text
BITS 32


global _isr128      ; syscall int 0x80

extern _isr_handler

; Makro für Exceptions ohne Error-Code
%macro ISR_NOERR 1
global _isr%1
_isr%1:
    cli
    push dword 0           ; Dummy error code
    push dword %1          ; Interrupt Nummer
    jmp _isr_common_stub
%endmacro

; Makro für Exceptions mit Error-Code (z.B. #PF, #GP)
%macro ISR_ERR 1
global _isr%1
_isr%1:
    cli
    push dword %1          ; Interrupt Nummer
    jmp _isr_common_stub
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30
ISR_NOERR 31

; Syscall (int 0x80) – kein Error-Code
_isr128:
    cli
    push dword 0          ; Dummy error code
    push dword 128
    jmp _isr_common_stub

_isr_common_stub:
    pusha                  ; alle GP-Register sichern

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10           ; unser Kernel-Data-Segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp               ; Zeiger auf regs-Struktur
    call _isr_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    popa                   ; GP-Regs wiederherstellen

    add esp, 8             ; error_code + int_no entfernen

    sti
    iretd
