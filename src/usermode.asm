BITS 32
section .text
global _enter_usermode
extern _user_mode_main
extern _user_stack_top

_enter_usermode:
    cli

    ; User-Data-Segment (Index 4, RPL=3 → 0x23)
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; User-Stack vorbereiten
    mov eax, [_user_stack_top]

    ; Stack-Frame für iret aufbauen:
    ; SS, ESP, EFLAGS, CS, EIP
    push dword 0x23         ; SS (User Data, RPL=3)
    push eax                ; ESP (User Stack Top)
    
    pushfd                 ; EFLAGS
    sti
    call _user_mode_main
    ret
