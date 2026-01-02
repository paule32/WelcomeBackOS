%define SCREEN_COLS 80
%define SCREEN_ROWS 25
%define VIDSEG      0xB800

%macro PRINT_AT 2
    mov di, (%1*80 + 6)*2
    mov si, %2
    mov bl, 0x20
    call print_base_str
%endmacro

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

    ; SHARE WARE text
    PRINT_AT 19, share1
    PRINT_AT 20, share2
    PRINT_AT 21, share3
    PRINT_AT 22, share4
    PRINT_AT 23, share5

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

    cmp ah, 4
    je .print_optionB4
    
    cmp ah, 1
    je .print_optionB1
    
    cmp ah, 2
    je .print_optionB2
    
    cmp ah, 3
    je .print_optionB3
    jmp main_bootmenu_loop
    
    .print_optionB3:
    mov bl, 0x2F
    call ui_draw_options_B
    mov ah, 2
    mov [menuFlag], ah
    jmp main_bootmenu_loop
    
    .print_optionB2:
    mov bl, 0x2F
    call ui_draw_options_A
    mov ah, 1
    mov [menuFlag], ah
    jmp main_bootmenu_loop

    .print_optionB4:
    .print_optionB1:
    mov bl, 0x2F
    call ui_draw_options_C
    mov ah, 3
    mov [menuFlag], ah
    jmp main_bootmenu_loop
    
on_down:
    call ui_draw_texte
    mov ah, [menuFlag]
    
    cmp ah, 4
    je .print_optionAdown0
    
    cmp ah, 1
    je .print_optionAdown1
    
    cmp ah, 2
    je .print_optionAdown2
    
    cmp ah, 3
    je .print_optionAdown3
    jmp main_bootmenu_loop
    
    .print_optionAdown0:
    .print_optionAdown3:
    mov bl, 0x2F
    call ui_draw_options_A
    mov ah, 1
    mov [menuFlag], ah
    jmp main_bootmenu_loop

    .print_optionAdown1:
    mov bl, 0x2F
    call ui_draw_options_B
    mov ah, 2
    mov [menuFlag], ah
    jmp main_bootmenu_loop
    
    .print_optionAdown2:
    mov bl, 0x2F
    call ui_draw_options_C
    mov ah, 3
    mov [menuFlag], ah
    jmp main_bootmenu_loop
    
on_enter:
    mov ah, [menuFlag]
    cmp ah, 1
    je .enter_text_pm
    
    cmp ah, 2
    je .enter_vesa_800x600_pm
    jmp main_bootmenu_loop

    .enter_vesa_800x600_pm:
    
    call dpa_packet_jump
    
    ; i see no output -->
    mov si, msgB2Fail
    call print_string
    <--
    
    
    jmp 0x08:pm_entry
    jmp $
    
    .enter_text_pm:
    call dpa_packet_jump
    jmp 0x08:pm_entry
    jmp $

dpa_packet_jump:
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
    
    ; -------------------------------------------------
    ; Kernel per LBA nach 0x8000:0000 laden
    ; -------------------------------------------------
    mov word  [dap_sectors ], KERNEL_SECTORS   ; aus xorriso
    mov word  [dap_offset  ], 0x0000          ; Offset 0
    mov word  [dap_segment ], 0x8000          ; Segment 0x8000 -> phys 0x80000
    mov dword [dap_lba_low ], KERNEL_LBA
    mov dword [dap_lba_high], 0

    ; Kernel nach physisch 0x00080000 laden
    mov ax, 0x8000
    mov es, ax          ; ES:BX = 8000:0000 → phys 0x80000
    mov bx, 0x0000
    
    mov si, disk_address_packet
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc .load_error

    mov si, msgB2OK
    call print_string

    jmp .lba_kernel_ok
    
    
.load_error:
    mov si, msgB2Fail
    call print_string

    ; Debug: Fehlercode ausgeben
    mov al, ah             ; AH = BIOS Error Code
    call print_hex8        ; z.B. " AH=0E"

.halt:
    cli
    hlt
    jmp .halt

.lba_kernel_ok:
    ; -------------------------------------------------
    ; A20 aktivieren
    ; -------------------------------------------------
    call enable_a20
    call check_a20
    jz a20_failed2          ; ZF=1 -> aus

    jmp A20ok2
    
    a20_failed2:
    sti
    mov si, msgA20FAIL
    call print_string
    hlt
    
    A20ok2:
    mov ah, [menuFlag]
    cmp ah, 2                       ; wenn grafik
    je  .set_graphics_mode_800x600   ; dann ...
    jne .only_text
    
    .set_graphics_mode_800x600:
    call get_vesa_mode
    call set_vesa_mode
    jmp switcher
    
    .only_text:
    ; -------------------------------------------------
    ; GDT laden, Protected Mode aktivieren
    ; -------------------------------------------------
    mov si, vessatext_a
    call print_string
    
    switcher:
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or  eax, 1             ; PE-Bit setzen
    mov cr0, eax
    ret

BITS 32
pm_entry:
    ; Segmente im Protected Mode setzen
    mov ax, 0x10          ; Data-Segment-Selector
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    
    mov esp, 0x0009F000   ; irgendein 32-Bit-Stack im oberen Bereich
    
    mov ah, [menuFlag]
    cmp ah, 1
    je .go_text
    cmp ah, 2
    je .go_graphics
    jmp main_bootmenu_loop

    ; entrypoint steht als dword am Anfang des
    ; geladenen kernel.bin (kernel.ld)
    .go_text:
    ; base = kernel load addr
    mov esi, 0x00080000
    ; wähle entry: 0=_text_main, 1=_graphics_main
    mov ebx, 0              ; 0 => _text_main
    mov eax, [esi + ebx*4]  ; dword lesen
    jmp eax
    jmp $

    ; entrypoint steht als dword am Anfang des
    ; geladenen kernel.bin (kernel.ld)
    .go_graphics:
    ; base = kernel load addr
    mov esi, 0x00080000
    ; wähle entry: 0=_text_main, 1=_graphics_main
    mov ebx, 1              ; 1 => _graphics_main
    mov eax, [esi + ebx*4]  ; dword lesen
    jmp eax
    jmp $
    