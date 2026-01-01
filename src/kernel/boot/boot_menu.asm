%define SCREEN_COLS 80
%define SCREEN_ROWS 25
%define VIDSEG      0xB800

boot_menu:
    ; Textmode 03h
    mov ax, 0x0003
    int 0x10

    ; Hintergrund pattern füllen
    mov al, 0xb1                ; pattern char
    mov ah, 0x1F               ; attr: white on blue (Beispiel)
    call ui_fill_bg

    mov ax, 0xB800
    mov es, ax
    
    ; top
    mov di, (5*80 + 10) * 2
    mov ax, 0x1ECD
    mov cx, 60
    .draw1:
    stosw
    loop .draw1
    
    ; bottom
    mov di, (15*80 + 10) * 2
    mov ax, 0x1ECD
    mov cx, 60
    .draw2:
    stosw
    loop .draw2

    ; left
    mov di, (6*80 + 10) * 2
    mov ax, 0x1EBA       ; AX = attr+char (AH=Attr, AL=Char)
    mov cx, 9            ; counter
    .draw3:
    stosw                ; schreibt an ES:DI und DI += 2
    add di, 160-2        ; nächste Zeile gleiche Spalte: +160 Bytes (80*2)
    loop .draw3
    
    ; right
    mov di, (6*80 + 69) * 2
    mov ax, 0x1EBA       ; AX = attr+char (AH=Attr, AL=Char)
    mov cx, 9            ; counter
    .draw4:
    stosw                ; schreibt an ES:DI und DI += 2
    add di, 160-2        ; nächste Zeile gleiche Spalte: +160 Bytes (80*2)
    loop .draw4
    
    ; links oben (Ecke)
    mov ax, 0xB800
    mov ds, ax
    mov di, (5*80 + 10)*2   ; y=5, x=10
    mov ax, 0x1EC9
    mov [di], ax
    
    ; links unten (Ecke)
    mov ax, 0xB800
    mov ds, ax
    mov di, (15*80 + 10)*2   ; y=15, x=10
    mov ax, 0x1EC8
    mov [di], ax
    
    ; rechts unten (Ecke)
    mov ax, 0xB800
    mov ds, ax
    mov di, (15*80 + 69)*2   ; y=15, x=69
    mov ax, 0x1EBC
    mov [di], ax
    
    ; rechts oben (Ecke)
    mov ax, 0xB800
    mov ds, ax
    mov di, (5*80 + 69)*2    ; y=5, x=10
    mov ax, 0x1EBB
    mov [di], ax

    ; hintergrund - innen
    mov ax, 0xB800
    mov es, ax
    cld
    mov di, (6*80 + 11)*2   ; Start: y=6, x=11
    mov ax, 0x1F20          ; ' ' (0x20) mit Attr 0x1F
    mov bx, 9               ; 9 Zeilen
    .row:
        mov cx, 58          ; 58 Zeichen pro Zeile
        rep stosw           ; füllt eine Zeile
        add di, 160 - 58*2  ; nächste Zeile: +160 bytes minus bereits geschriebene 58 bytes
        dec bx
        jnz .row
    
    ; dbase (mitte)
    mov ax, 0xB800
    mov ds, ax
    mov di, (5*80 + 29)*2   ; y=15, x=69
    mov ax, 0x1EBC
    mov [di], ax
    ; ----
    mov ax, 0xB800
    mov ds, ax
    mov di, (5*80 + 50)*2   ; y=15, x=69
    mov ax, 0x1EBC
    mov [di], ax
    ;
    mov ax, 0xB800
    mov ds, ax
    mov di, (4*80 + 29)*2   ; y=15, x=69
    mov ax, 0x1EBA
    mov [di], ax
    ; ----
    mov ax, 0xB800
    mov ds, ax
    mov di, (4*80 + 50)*2   ; y=15, x=69
    mov ax, 0x1EBA
    mov [di], ax
    ;
    
    ; dbase logo
    push cs
    pop ds
    mov ax, 0xB800
    mov es, ax
    mov di, (4*80 + 30)*2   ; y=6, x=11
    mov ah, 0x1F
    mov si, msgDBASE
    cld
    .loop:
    lodsb
    test al, al
    jz .endeDBstr
    stosw
    jmp .loop
    .endeDBstr:
    
    ; linke Ecke
    mov ax, 0xB800
    mov ds, ax
    mov di, (3*80 + 29)*2   ; y=15, x=69
    mov ax, 0x1EC9
    mov [di], ax
    
    ; rechte Ecke
    mov ax, 0xB800
    mov ds, ax
    mov di, (3*80 + 50)*2   ; y=15, x=69
    mov ax, 0x1EBB
    mov [di], ax
    
    ; balken-mitte (oben)
    ; top
    mov di, (3*80 + 30) * 2
    mov ax, 0x1ECD
    mov cx, 20
    .draw5:
    stosw
    loop .draw5
    
    ; text unten
    push cs
    pop ds
    mov ax, 0xB800
    mov es, ax
    mov di, (15*80 + 22)*2   ; y=6, x=11
    mov ah, 0x1F
    mov si, msgDBASEenv
    cld
    .loop2:
    lodsb
    test al, al
    jz .endeDBstr2
    stosw
    jmp .loop2
    .endeDBstr2:

    ; balken-mitte unten (unten)
    mov di, (16*80 + 21) * 2
    mov ax, 0x1ECD
    mov cx, 36
    .draw51:
    stosw
    loop .draw51
    ; balken-mitte unten (oben)
    mov di, (14*80 + 21) * 2
    mov ax, 0x1ECD
    mov cx, 36
    .draw52:
    stosw
    loop .draw52

mov di, (19*80 + 6)*2   ; y=6, x=11
mov si, share1
mov bl, 0x20
call print_base_str
;
mov di, (20*80 + 6)*2   ; y=6, x=11
mov si, share2
mov bl, 0x20
call print_base_str
;
mov di, (21*80 + 6)*2   ; y=6, x=11
mov si, share3
mov bl, 0x20
call print_base_str
;
mov di, (22*80 + 6)*2   ; y=6, x=11
mov si, share4
mov bl, 0x20
call print_base_str
;
mov di, (23*80 + 6)*2   ; y=6, x=11
mov si, share5
mov bl, 0x20
call print_base_str
;

    call ui_draw_texte
    call main_bootmenu_loop
jmp $
    ret

ui_draw_texte:
    mov bl, 0x1F
    call ui_draw_options_A
    ;
    mov bl, 0x1F
    call ui_draw_options_B
    ;
    mov bl, 0x1F
    call ui_draw_options_C
    ret
    
ui_draw_options_A:
    ; starten im Textmode 80x25
    push cs
    pop ds
    mov ax, 0xB800
    mov es, ax
    mov di, (7*80 + 16)*2   ; y=6, x=11
    mov ah, bl
    mov si, msgDBASEtextmode80x25
    cld
    .loop:
    lodsb
    test al, al
    jz .endeDBstr
    stosw
    jmp .loop
    .endeDBstr:
    ret

ui_draw_options_B:
    ; starten im vesa mode 800x600x16bpp
    push cs
    pop ds
    mov ax, 0xB800
    mov es, ax
    mov di, (9*80 + 16)*2   ; y=6, x=11
    mov ah, bl
    mov si, msgDBASEvesa800x600
    cld
    .loop:
    lodsb
    test al, al
    jz .endeDBstr
    stosw
    jmp .loop
    .endeDBstr:
    ret

ui_draw_options_C:
    ; starten im vesa mode 1024x728x16bpp
    push cs
    pop ds
    mov ax, 0xB800
    mov es, ax
    mov di, (11*80 + 16)*2   ; y=6, x=11
    mov ah, bl
    mov si, msgDBASEvesa1024x728
    cld
    .loop:
    lodsb
    test al, al
    jz .endeDBstr
    stosw
    jmp .loop
    .endeDBstr:
    ret

print_base_str:
    push cs
    pop ds
    mov ax, 0xB800
    mov es, ax
    mov ah, bl
    cld
    .loop:
    lodsb
    test al, al
    jz .endeDBstr
    stosw
    jmp .loop
    .endeDBstr:
    ret

; ------------------------------------------------------------
; UI: Fill screen with char in AL and attr in AH
; ------------------------------------------------------------
ui_fill_bg:
    push ax
    push bx
    push cx
    push di
    mov bx, VIDSEG
    mov es, bx
    xor di, di
    mov cx, SCREEN_COLS*SCREEN_ROWS
.fill:
    stosw
    loop .fill
    pop di
    pop cx
    pop bx
    pop ax
    ret

; ------------------------------------------------------------
; BIOS Keyboard Loop:
; - wartet auf Taste
; - reagiert auf UP / DOWN / ENTER
; ------------------------------------------------------------
main_bootmenu_loop:
    ; Check ob Taste verfügbar (ZF=1 => keine Taste)
    mov ah, 0x01
    int 0x16
    jz  main_bootmenu_loop

    ; Taste lesen: AH=scancode, AL=ascii (bei Pfeiltasten AL=0 oder 0xE0)
    mov ah, 0x00
    int 0x16

    ; ENTER?
    cmp ah, 0x1C
    je  on_enter

    ; UP?
    cmp ah, 0x48
    je  on_up

    ; DOWN?
    cmp ah, 0x50
    je  on_down

    jmp main_bootmenu_loop

on_up:
    call ui_draw_texte
    mov ah, [menuFlag]

    cmp ah, 3
    je .print_optionBup
    
    cmp ah, 2
    je .print_optionAup
    
    cmp ah, 1
    je .print_optionCup
    jmp main_bootmenu_loop
    
    .print_optionAup:
    mov bl, 0x2F
    call ui_draw_options_A
    mov ah, 1
    mov [menuFlag], ah
    jmp main_bootmenu_loop

    .print_optionBup:
    mov bl, 0x2F
    call ui_draw_options_B
    mov ah, 2
    mov [menuFlag], ah
    jmp main_bootmenu_loop

    .print_optionCup:
    mov bl, 0x2F
    call ui_draw_options_C
    mov ah, 3
    mov [menuFlag], ah
    jmp main_bootmenu_loop
    
on_down:
    call ui_draw_texte
    mov ah, [menuFlag]
    
    cmp ah, 1
    je .print_optionAdown
    
    cmp ah, 2
    je .print_optionBdown
    
    cmp ah, 3
    je .print_optionCdiwn
    jmp main_bootmenu_loop
    
    .print_optionCdiwn:
    mov bl, 0x2F
    call ui_draw_options_C
    mov ah, 1
    mov [menuFlag], ah
    jmp main_bootmenu_loop
    
    .print_optionBdown:
    mov bl, 0x2F
    call ui_draw_options_B
    mov ah, 3
    mov [menuFlag], ah
    jmp main_bootmenu_loop
    
    .print_optionAdown:
    mov bl, 0x2F
    call ui_draw_options_A
    mov ah, 2
    mov [menuFlag], ah
    jmp main_bootmenu_loop

on_enter:
    ; TODO: bestätigen / weiter
    jmp main_bootmenu_loop
    
; ------------------------------------------------------------
; Daten
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

menuFlag db 1
