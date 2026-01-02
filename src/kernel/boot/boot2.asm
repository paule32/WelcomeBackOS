; ---------------------------------------------------------------------------
; \file  boot2.asm
; \note  (c) 2025, 2026 by Jens Kallup - paule32
;        all rights reserved.
; ---------------------------------------------------------------------------
; boot2.asm – Minimaler Stage2
%include LBA_FILE

%ifndef ISOGUI
    %define ISOGUI 0    ; default text input
%endif

BITS 16
ORG 0x0500          ; Stage1 springt nach 0000:0500

start_boot2:
    cli
    
    ; Code- und Datensegmente angleichen
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x5000      ; irgendein Stack im ersten MB
    
    sti

    mov [boot_drive], dl

    mov si, msgB2Start
    call print_string
    
    ; -------------------------------------------------
    ; Text-Interface für Benutzer-Desktop text or gui
    ; -------------------------------------------------
    %include 'src/kernel/boot/boot_menu.asm'
    call boot_menu
    ; no return here

;---------------------------------------------------------
; Versuch 1: BIOS-Funktion INT 15h, AX=2401h
;---------------------------------------------------------
enable_a20:
    ; 1) Fast A20 (Port 0x92)
    in   al, 0x92
    or   al, 00000010b
    and  al, 11111110b
    out  0x92, al
    
    mov ax, 0x2401
    int 0x15
    jc .via_kbd   ; wenn Fehler: Keyboard-Controller-Methode

    ret

    .via_kbd:
    ; Sehr vereinfachte KBC-Methode (reicht fuer QEMU / Bochs)
    .wait_input:
    in al, 0x64
    test al, 2
    jnz .wait_input

    mov al, 0xD1
    out 0x64, al

    .wait_input2:
    in al, 0x64
    test al, 2
    jnz .wait_input2

    mov al, 0xDF         ; A20 einschalten
    out 0x60, al

    ret

;---------------------------------------------------------
; print_string: DS:SI -> 0-terminierte Zeichenkette
;---------------------------------------------------------
print_string:
    lodsb
    cmp al, 0
    je .ps_done
    mov ah, 0x0E
    mov bh, 0
    mov bl, 0x07
    int 0x10
    jmp print_string
.ps_done:
    ret

;---------------------------------------------------------
; print_hex8: AL -> „0xHH“ ausgeben
;---------------------------------------------------------
print_hex8:
    push ax
    push bx
    push cx
    push dx

    mov si, msgAH         ; " AH="
    call print_string

    mov ah, al            ; AL = Wert, AH = Kopie
    shr al, 4             ; high nibble
    call print_hex_nibble

    mov al, ah            ; low nibble
    and al, 0x0F
    call print_hex_nibble
    
    mov al, 13
    mov ah, 0x0E
    int 0x10
    mov al, 10
    mov ah, 0x0E
    int 0x10

    pop dx
    pop cx
    pop bx
    pop ax
    ret

; AL = 0..15 -> '0'..'9','A'..'F'
print_hex_nibble:
    cmp al, 10
    jb digit
    add al, 'A' - 10
    jmp _out
digit:
    add al, '0'
_out:
    mov ah, 0x0E
    mov bh, 0
    mov bl, 0x07
    int 0x10
    ret

; returns ZF=0 wenn A20 AN, ZF=1 wenn A20 AUS
check_a20:
    pushf
    cli
    push ds
    push es
    xor ax, ax
    mov ds, ax          ; DS = 0x0000
    mov ax, 0xFFFF
    mov es, ax          ; ES = 0xFFFF

    mov si, 0x0500      ; 0000:0500  -> phys 0x000500
    mov di, 0x0510      ; FFFF:0510  -> phys 0x100500 (wenn A20 an)

    mov al, [ds:si]
    mov bl, [es:di]

    mov byte [ds:si], 0x00
    mov byte [es:di], 0xFF

    cmp byte [ds:si], 0xFF   ; wenn A20 AUS -> wrap -> wird 0xFF
    ; restore
    mov [ds:si], al
    mov [es:di], bl

    pop es
    pop ds
    popf
    ret
    
; -------------------------------------------------
; VESA Mode 0x114 mit LFB setzen und Infos sichern
; -------------------------------------------------
get_vesa_mode:
    ;%ifdef ISOGUI = 0
    ;ret                   ; text interface verwenden
    ;%endif
     
    ; 1) Mode-Info holen
    mov ax, 0x4F01        ; VBE-Funktion: Get Mode Info
    mov cx, 0x0114        ; gewünschter Modus: 0x114
    xor bx, bx
    mov es, bx            
    mov di, 0x2000        ; ES -> 0x2000 -> ES:DI = 0000:2000 (phys 0x00002000)
    int 0x10
    
    cmp ax, 0x004F
    jne vbe_fail

set_vesa_mode:
    mov ax, 0x4F02        ; VBE-Funktion: Set VBE Mode
    mov bx, 0x4114        ; 0x114 | 0x4000 (Bit14 = Linear Framebuffer)
    int 0x10

    cmp ax, 0x004F
    jne vbe_fail

    ; Erfolg: hier geht's normal weiter (z.B. in Protected Mode wechseln)
    jmp vbe_ok

    ; optional: Debug: "VESA OK" ausgeben
    ; ...
vbe_ok:
    ret

vbe_fail:
    ; hier fallback z.B. Textmodus lassen, Meldung zeigen
    ; ...
    mov si, msgVESAerr
    call print_string
vbe_hlt:
    hlt
    jmp vbe_hlt

;---------------------------------------------------------
; Daten
;---------------------------------------------------------
BITS 16
boot_drive      db 0

msgB2Start      db 13,10,"[Stage2] BOOT2 gestartet, Kernel wird geladen ...",13,10,0
msgB2OK         db "[Stage2] Kernel geladen, springe nach 8000:0000",13,10,0
msgA20OK        db "[Stage2] A20 aktiviert, Protected Mode wird gestartet ...",13,10,0
msgA20FAIL      db "[Stage2] A20 Fehler",13,10,0
msgB2Fail       db "[Stage2] FEHLER beim Laden des Kernels",13,10,0
msgBeforePM     db "[Stage2] Protected Mode CLI",13,10,0
msgVESAerr      db "[Stage2] VESA Modus Fehler!",13,10,0

msgAH           db " AH=",0

; einfache 3-Entry-GDT: null, code, data
gdt_start:
gdt_null:   dq 0
gdt_code:   dq 0x00CF9A000000FFFF   ; base=0, limit=4GB, Code, RX
gdt_data:   dq 0x00CF92000000FFFF   ; base=0, limit=4GB, Data, RW
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

;---------------------------------------------------------
; Disk Address Packet
;---------------------------------------------------------
disk_address_packet:
dap_size        db 0x10
dap_reserved    db 0
dap_sectors     dw 0
dap_offset      dw 0
dap_segment     dw 0
dap_lba_low     dd 0
dap_lba_high    dd 0

; ------------------------------------------------------------
; boot menu
; ------------------------------------------------------------
msgDBASEtextmode80x25:  db " Start dBase 2026 Text-Mode 80x25        ... ", 0
msgDBASEvesa800x600:    db " Start dBase 2026 Graphics-Mode  800x600 ... ", 0
msgDBASEvesa1024x728:   db " Start dBase 2026 Graphics-Mode 1024x728 ... ", 0
; ------------------------------------------------------------
msgDBASE:               db " -=< dBASE 2026 >=- ",0
msgDBASEenv:            db " Choose your Favorite Environment ", 0

share1: db "  ____  _   _    _    ____  _____      _    _    _    ____  _____  ", 0
share2: db " / ___|| | | |  / \  |  _ \| ____|    | |  | |  / \  |  _ \| ____| ", 0
share3: db " \___ \| |_| | / _ \ | |_) |  _|      | |/\| | / _ \ | |_) |  _|   ", 0
share4: db "  ___) |  _  |/ ___ \|  _ <| |___     |  /\  |/ ___ \|  _ <| |___  ", 0
share5: db " |____/|_| |_/_/   \_\_| \_\_____|    |_|  |_/_/   \_\_| \_\_____| ", 0

vessatext_a: db "vESSA 1111", 0
vessatext_b: db "vESSA CCCC", 0
vessatext_c: db "gugu ", 0
menuFlag db 4

; Puffer für VBE Mode Info (256 Byte reichen)
vesa_mode_info:
    times 256 db 0
