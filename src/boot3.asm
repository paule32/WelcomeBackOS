BITS 16
ORG 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    cld
    sti

    mov si, msg1
    call print_string

.hang:
    jmp .hang

print_string:
    mov ah, 0x0E
.next:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .next
.done:
    ret

msg1 db 13,10,'*** BOOTTEST: Ich laufe! ***',13,10,0

times 510-($-$$) db 0
dw 0xAA55
