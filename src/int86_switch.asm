; int86_trampoline.asm
; Trampoline wird nach 0x7000 kopiert und dort ausgeführt (identity mapped)
; Mailbox liegt bei 0x6000 (identity mapped)

%define MAILBOX      0x00006000
%define RM_STACK     0x00006FF0

; Mini-GDT Selektoren (innerhalb dieses Trampolines!)
%define T_SEL_CODE32  0x08
%define T_SEL_DATA32  0x10
%define T_SEL_CODE16  0x18
%define T_SEL_DATA16  0x20

bits 32
org 0x7000

tramp32:
    cli

    ; -------- Paging AUS (wir sind im Lowmem) --------
    mov eax, cr0
    and eax, 0x7FFFFFFF           ; clear PG
    mov cr0, eax

    ; kleine Serialisierung (optional aber ok)
    jmp short $+2

    ; -------- In 16-bit Protected Mode (Trampoline-GDT) --------
    lgdt [t_gdt_ptr]              ; Mini-GDT laden
    jmp T_SEL_CODE16:pm16_entry

; ============================================================
; 16-bit Protected Mode
; ============================================================
bits 16
pm16_entry:
    mov ax, T_SEL_DATA16
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, RM_STACK

    ; Real Mode IVT (0:0) als IDT nutzen
    lidt [rm_idtr]

    ; Protected Mode AUS
    mov eax, cr0
    and eax, 0xFFFFFFFE           ; clear PE
    mov cr0, eax

    ; Pipeline flush
    jmp 0x0000:rm_entry

; ============================================================
; Real Mode
; ============================================================
bits 16
rm_entry:
    ; RM segmente setzen
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, RM_STACK

    ; -------- Input-REGS aus Mailbox laden --------
    ; Layout:
    ; MAILBOX+0 intno
    ; MAILBOX+1 status
    ; MAILBOX+4 in (REGS16)
    mov si, MAILBOX + 4

    mov ax, [si + 0]      ; AX
    mov bx, [si + 2]      ; BX
    mov cx, [si + 4]      ; CX
    mov dx, [si + 6]      ; DX
    mov di, [si + 10]     ; DI
    mov bp, [si + 12]     ; BP
    mov ds, [si + 14]     ; DS
    mov es, [si + 16]     ; ES
    mov si, [si + 8]      ; SI zuletzt

    ; Interruptnummer holen
    mov bl, [MAILBOX + 0]

    ; -------- BIOS INT Dispatch --------
    cmp bl, 0x10
    je .do10
    cmp bl, 0x13
    je .do13
    cmp bl, 0x15
    je .do15
    cmp bl, 0x16
    je .do16
    cmp bl, 0x1A
    je .do1A

    ; unsupported:
    mov byte [MAILBOX + 1], 1
    stc
    jmp .after_int

.do10: int 0x10
       jmp .after_int
.do13: int 0x13
       jmp .after_int
.do15: int 0x15
       jmp .after_int
.do16: int 0x16
       jmp .after_int
.do1A: int 0x1A

.after_int:
    ; -------- Output-REGS + FLAGS sichern --------
    ; Wichtig: erst sichern, dann irgendwo Pointer benutzen.
    pushf
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    push bp
    push ds
    push es

    ; out liegt bei MAILBOX+4 + sizeof(in)=20 bytes => MAILBOX+24
    mov di, MAILBOX + 4 + 20

    pop word [di + 16]    ; ES
    pop word [di + 14]    ; DS
    pop word [di + 12]    ; BP
    pop word [di + 10]    ; DI
    pop word [di + 8]     ; SI
    pop word [di + 6]     ; DX
    pop word [di + 4]     ; CX
    pop word [di + 2]     ; BX
    pop word [di + 0]     ; AX
    pop word [di + 18]    ; FLAGS

    ; -------- Zurück nach Protected Mode --------
    cli
    lgdt [t_gdt_ptr]

    mov eax, cr0
    or  eax, 1            ; set PE
    mov cr0, eax
    jmp T_SEL_CODE32:pm32_entry

; ============================================================
; 32-bit Protected Mode (Trampoline)
; ============================================================
bits 32
pm32_entry:
    mov ax, T_SEL_DATA32
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Paging wieder AN
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    ret

; ============================================================
; RM IDTR für IVT @ 0:0
; ============================================================
bits 16
rm_idtr:
    dw 0x03FF
    dd 0x00000000

; ============================================================
; Mini-GDT (liegt im Trampoline, funktioniert ohne externes GDT)
; Base=0 Limit=4GiB, einmal 32-bit und einmal 16-bit Segmente
; ============================================================
bits 32
align 8
t_gdt:
    dq 0x0000000000000000         ; null

    ; 0x08: code32 base=0 limit=4GiB, D=1
    dq 0x00CF9A000000FFFF

    ; 0x10: data32 base=0 limit=4GiB, D=1
    dq 0x00CF92000000FFFF

    ; 0x18: code16 base=0 limit=64KiB, D=0
    dq 0x000F9A000000FFFF

    ; 0x20: data16 base=0 limit=64KiB, D=0
    dq 0x000F92000000FFFF

t_gdt_end:

t_gdt_ptr:
    dw t_gdt_end - t_gdt - 1
    dd t_gdt
