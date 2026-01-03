; ---------------------------------------------------------------------------
; \file  boot2.asm
; \note  (c) 2025 by Jens Kallup - paule32
;        all rights reserved.
; ---------------------------------------------------------------------------
; boot2.asm – Minimaler Stage2
; ---------------------------------------------------------------------------
BITS 16
ORG 0x0500          ; Stage1 springt nach 0000:0500
jmp start

%include LBA_FILE

%ifndef ISOGUI
    %define ISOGUI 0    ; default text input
%endif

%define CHUNK_SECTORS    15

%define LOAD_SEG         0x8000        ; ES=9000, BX=0000 => phys 0x00080000
%define LOAD_OFF         0x0000
%define LOAD_BUF_PHYS    0x00080000

%define KERNEL_DST_BASE  0x00400000    ; 4 MiB Zieladresse

%define SCREEN_COLS      80
%define SCREEN_ROWS      25
%define VIDSEG           0xB800

%define USE_ENTRY_PTR    1   ; 1: DWORD [0x00400000] ist Entry
                             ; 0: direkt jmp 0x00400000
%macro PRINT_AT 2
    mov di, (%1*80 + 6)*2
    mov si, %2
    mov bl, 0x20
    call print_base_str
%endmacro

check_edd:
    mov dl, [boot_drive]
    mov ax, 0x4100
    mov bx, 0x55AA
    int 0x13
    jc  .no
    cmp bx, 0xAA55
    jne .no
    test cx, 1          ; Bit0: Extensions vorhanden
    jz  .no
    ret
.no:
    ; hier Fehlertext ausgeben und hängen
    jmp disk_error
    
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Falls Stage1 DL übergibt, kannst du hier speichern:
    mov [boot_drive], dl
    call check_edd

    ; Textmodus sicher
    ;mov ax, 0x0003
    ;int 0x10

    push cs
    pop  ds

    mov si, msgB2Start
    call print_string

    call enable_a20_fast

    ; Laden + Kopieren nach 4 MiB
    call load_kernel_chunks_to_4mb

    ; Jetzt sind wir wieder im RM. Status anzeigen:
    push cs
    pop  ds
    mov si, msg_loaded
    call print_string

    ; Final in Protected Mode und Kernel starten
    call enter_pm_final

[BITS 32]
%if USE_ENTRY_PTR
    mov eax, dword [KERNEL_DST_BASE]    ; Entry-Pointer am Image-Anfang
    jmp eax
%else
    jmp KERNEL_DST_BASE                 ; direkter Entry bei 0x00400000
%endif

; ------------------------------------------------------------
; print_string: DS:SI -> 0-terminierte Zeichenkette (BIOS teletype)
; ------------------------------------------------------------
[BITS 16]
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

; ------------------------------------------------------------
; Kernel in Chunks lesen und je Chunk nach 0x00400000 kopieren
; ------------------------------------------------------------
[BITS 16]
load_kernel_chunks_to_4mb:
    mov word [sectors_left], KERNEL_SECTORS
    mov word [sectors_done], 0

.next_chunk:
    ; -------->
    push cx
    push bx
    push ax
    push si
    push di
    push ds
    push es
    mov si, dotstr1      ; <--- load status
    call print_string
    pop es
    pop ds
    pop di
    pop si
    pop ax
    pop bx
    pop cx
    ; <--------

    mov ax, [sectors_left]
    test ax, ax
    jz .done

    ; chunk = min(sectors_left, CHUNK_SECTORS)
    mov cx, CHUNK_SECTORS
    cmp ax, cx
    jbe .use_ax
    mov ax, cx
.use_ax:
    ; AX = chunk sectors
    
    ; -------->
    push cx
    push bx
    push ax
    push si
    push di
    push ds
    push es
    mov si, dotstr2      ; <--- load status
    call print_string
    pop es
    pop ds
    pop di
    pop si
    pop ax
    pop bx
    pop cx
    ; <--------

    ; DAP füllen
    mov word  [dap_sectors], ax
    mov word  [dap_offset ], LOAD_OFF
    mov word  [dap_segment], LOAD_SEG

    ; LBA = KERNEL_LBA + sectors_done (nur low32)
    movzx eax, word [sectors_done]
    add eax, dword [kernel_lba_low]
    mov dword [dap_lba_low], eax
    mov dword [dap_lba_high], 0

    ; -------->
    push cx
    push bx
    push ax
    push si
    push di
    push ds
    push es
    mov si, dotstr3      ; <--- load status
    call print_string
    pop es
    pop ds
    pop di
    pop si
    pop ax
    pop bx
    pop cx
    ; <--------

    ; BIOS Read (EDD)
    push cs
    pop  ds
    mov si, disk_address_packet
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc  disk_error

    ; -------->
    push cx
    push bx
    push ax
    push si
    push di
    push ds
    push es
    mov si, dotstr4      ; <--- load status
    call print_string
    pop es
    pop ds
    pop di
    pop si
    pop ax
    pop bx
    pop cx
    ; <--------

    ; Chunk im PM kopieren (kommt garantiert in RM zurück!)
    call pm_copy_chunk_thunk

    ; bookkeeping
    mov ax, [sectors_left]
    sub ax, [dap_sectors]
    mov [sectors_left], ax

    mov ax, [sectors_done]
    add ax, [dap_sectors]
    mov [sectors_done], ax

    jmp .next_chunk

.done:
    ret

; ------------------------------------------------------------
; PM Copy Thunk: RM -> PM -> copy -> RM -> ret
; Verhindert "call/ret über Modusgrenze" Probleme.
; ------------------------------------------------------------
[BITS 16]
pm_copy_chunk_thunk:
    cli
    lgdt [gdt_desc]

    ; Protected Mode an
    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    ; Far jump flush
    jmp CODE_SEL:pm_copy_do

[BITS 32]
pm_copy_do:
    mov ax, DATA_SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Ziel = 0x00400000 + sectors_done*512
    movzx eax, word [sectors_done]
    shl eax, 9
    add eax, KERNEL_DST_BASE
    mov edi, eax

    mov esi, LOAD_BUF_PHYS

    ; Länge = dap_sectors*512 Bytes => dwords
    movzx ecx, word [dap_sectors]
    shl ecx, 9
    shr ecx, 2

    cld
    rep movsd

    ; zurück nach Real Mode
    cli
    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax
    
    jmp 0x0000:rm_after_pm_copy

; ------------------------------------------------------------
; Finaler Wechsel in PM (bleiben) und dann Kernel-Jump (oben)
; ------------------------------------------------------------
[BITS 16]
enter_pm_final:
    cli
    lgdt [gdt_desc]

    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    jmp CODE_SEL:pm_final_entry

[BITS 32]
pm_final_entry:
    mov ax, DATA_SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x0090000         ; Stack (Beispiel im Lowmem)
    ret

; ------------------------------------------------------------
; A20 schnell (Port 0x92)
; ------------------------------------------------------------
[BITS 16]
enable_a20_fast:
    in   al, 0x92
    or   al, 00000010b
    and  al, 11111110b
    out  0x92, al
    ret

; ------------------------------------------------------------
; Fehler
; ------------------------------------------------------------
[BITS 16]
; ---------------------------------------------------------
; print_error_from_table
;   IN:  AL = error code
;   OUT: gibt passenden String aus
; ---------------------------------------------------------
print_error_from_table:
    push bx
    push di
    
    mov di, disk_status_error_table ; DI zeigt auf aktuellen Eintrag

.next:
    mov bl, [di]                ; BL = code (db)
   cmp bl, 0xFF
   je  .not_found

    cmp bl, al
    je  .found

    add di, 3                   ; nächstes Paar (1+2 bytes)
    jmp .next

.found:
    mov si, [di+1]              ; SI = dw pointer (Offset im gleichen Segment)
    call print_string
    jmp .done

.not_found:
    mov si, [di+1]              ; unknown_error
    call print_string

.done:
    pop di
    pop bx
    ret
    
disk_error:
    push cs
    pop  ds
    
    mov si, msgStage2Error
    call print_string
    
    mov ah, 1
    mov dl, [boot_drive]
    int 0x13

    mov al, ah

    push cs
    pop  ds
    call print_error_from_table
    
.hang:
    cli
    hlt
    jmp .hang


; --------------------------------------------------------------------
; Protected Mode Enter/Leave (minimal, ohne IDT)
; Achtung: Während PM sind BIOS-Interrupts tabu => wir gehen pro Chunk zurück.
; --------------------------------------------------------------------
[BITS 16]
enter_pm:
    cli
    lgdt [gdt_desc]

    ; Protected Mode an
    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    ; far jump flush
    jmp CODE_SEL:pm_entry

[BITS 32]
pm_entry:
    mov ax, DATA_SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Zieladresse = 0x00400000 + sectors_done*512
    movzx eax, word [sectors_done]
    shl eax, 9
    add eax, KERNEL_DST_BASE
    mov edi, eax

    mov esi, LOAD_BUF_PHYS

    ; Länge = dap_sectors*512 Bytes => dwords
    movzx ecx, word [dap_sectors]
    shl ecx, 9
    shr ecx, 2

    cld
    rep movsd

    ; zurück nach Real Mode
    cli
    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax

    jmp 0x0000:rm_after_pm_copy
    ret
    
; ------------------ Wieder Real Mode ------------------
[BITS 16]
rm_after_pm_copy:
    ; Segmente wieder vernünftig
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    ; Stack wiederherstellen (oder deinen echten RM-Stack)
    mov sp, 0x7C00

    sti   ;
    ; -------->
    push cx
    push bx
    push ax
    push si
    push di
    push ds
    push es
    mov si, dotstr5      ; <--- load status
    call print_string
    pop es
    pop ds
    pop di
    pop si
    pop ax
    pop bx
    pop cx
    ; <--------
    cli

    ret

; Zurück nach Real Mode (für nächsten BIOS-Read)
leave_pm:
    cli
    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax
    jmp 0x0000:rm_entry

[BITS 16]
rm_entry:
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    ret

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

    ; menu bar
    mov di, (0*80 + 0) * 2
    mov ax, 0x07DB
    mov cx, 80
    .drawMenu:
    stosw
    loop .drawMenu
    ; status
    mov di, (24*80 + 0) * 2
    mov ax, 0x07DB
    mov cx, 80
    .drawStatus:
    stosw
    loop .drawStatus

    
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
    PRINT_AT 18, share1
    PRINT_AT 19, share2
    PRINT_AT 20, share3
    PRINT_AT 21, share4
    PRINT_AT 22, share5

    call ui_draw_texte
    call main_bootmenu_loop
jmp $
    ret

; ------------------------------------
; BIOS RTC Zeit lesen
; INT 1Ah / AH=02h
; ------------------------------------
get_rtc_time:
    mov ah, 0x02
    int 0x1A
    jc .error

    ; Rückgabewerte (BCD!)
    ; CH = Stunden
    ; CL = Minuten
    ; DH = Sekunden
    ret

.error:
    ret

; AL = BCD → zwei ASCII-Zeichen
bcd_to_ascii:
    push ax
    mov ah, al
    shr ah, 4          ; Zehner
    and al, 0x0F       ; Einer
    add ah, '0'
    add al, '0'
    pop ax
    ret
; Input : AL = BCD (z.B. 0x59)
; Output: AL = ASCII Zehner ('0'..'5')
;         AH = ASCII Einer  ('0'..'9')
bcd_to_2ascii:
    mov ah, al
    and ah, 0x0F      ; Einer
    shr al, 4         ; Zehner
    add al, '0'
    add ah, '0'
    ret
    
show_time:
    mov ah, 0x02
    int 0x1A
    jc .done

    mov ax, 0xB800
    mov es, ax
    mov di, (0*80 + 71)*2
    mov bl, 0x70

    ; Stunden (CH)
    mov al, ch
    call bcd_to_2ascii
    mov dl, ah          ; Einer sichern
    mov ah, bl
    stosw               ; Zehner
    mov al, dl
    stosw               ; Einer

    ; :
    mov al, ':'
    mov ah, bl
    stosw

    ; Minuten (CL)
    mov al, cl
    call bcd_to_2ascii
    mov dl, ah
    mov ah, bl
    stosw
    mov al, dl
    stosw

    ; :
    mov al, ':'
    mov ah, bl
    stosw

    ; Sekunden (DH)
    mov al, dh
    call bcd_to_2ascii
    mov dl, ah
    mov ah, bl
    stosw
    mov al, dl
    stosw

.done:
    ret

ui_draw_texte:
    call ui_text_init
    mov bl, 0x1F
    call ui_draw_options_A
    ;
    call ui_text_init
    mov bl, 0x1F
    call ui_draw_options_B
    ;
    call ui_text_init
    mov bl, 0x1F
    call ui_draw_options_C
    ret

ui_text_init:
    push cs
    pop  ds
    mov  ax, 0xB800
    mov  es, ax
    cld
    ret

; ------------------------------------------------------------
;   DI = Zielposition im Textspeicher (Wortoffset)
;   SI = String (0-terminiert)
;   AH = Attribut
; ------------------------------------------------------------
ui_puts_at:
.next:
    lodsb
    or   al, al
    jz   .done
    stosw
    jmp  short .next
.done:
    ret

ui_draw_options_A:
    mov  di, (7*80 + 16)*2
    mov  ah, bl
    mov  si, msgDBASEtextmode80x25
    jmp  ui_puts_at
    ret
ui_draw_options_B:
    mov  di, (9*80 + 16)*2
    mov  ah, bl
    mov  si, msgDBASEvesa800x600
    jmp  ui_puts_at
    ret
ui_draw_options_C:
    mov  di, (11*80 + 16)*2
    mov  ah, bl
    mov  si, msgDBASEvesa1024x728
    jmp  ui_puts_at
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
    call show_time
    
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
    je enter_text_pm
    
    cmp ah, 2
    je enter_vesa_800x600_pm
    
    cmp ah, 3
    je main_bootmenu_loop
    
    cmp ah, 4
    je enter_vesa_800x600_pm
    
    jmp main_bootmenu_loop

enter_vesa_800x600_pm:
    ; -------------------------------------------------
    ; A20 aktivieren
    ; -------------------------------------------------
    call enable_a20
    call check_a20
    jz a20_failed          ; ZF=1 -> aus

    mov si, msgA20OK
    call print_string
    jmp A20ok
    
    a20_failed:
    mov si, msgA20FAIL
    call print_string
    ;jmp enter_vesa_800x600_pm
    
    A20ok:
    sti
    call set_vesa_mode_800x600

    ; -------------------------------------------------
    ; GDT laden, Protected Mode aktivieren
    ; -------------------------------------------------
    mov si, msgBeforePM
    call print_string
    
    cli
    ;lgdt [gdt_descriptor]
    
    mov eax, cr0
    or  eax, 1             ; PE-Bit setzen
    mov cr0, eax

    ; Far Jump in 32-Bit-Code (pm_entry)
    jmp 0x08:pm_entry      ; 0x08 = Code-Segment-Selector

    
    
    
enter_text_pm:
    ; -------------------------------------------------
    ; A20 aktivieren
    ; -------------------------------------------------
    call enable_a20
    call check_a20
    jz a20_failed2          ; ZF=1 -> aus

    mov si, msgA20OK
    call print_string
    jmp A20ok2
    
    a20_failed2:
    mov si, msgA20FAIL
    call print_string
    hlt
    
    ; -------------------------------------------------
    ; GDT laden, Protected Mode aktivieren
    ; -------------------------------------------------
    A20ok2:
    cli
    ;lgdt [gdt_descriptor]
    
    mov eax, cr0
    or  eax, 1             ; PE-Bit setzen
    mov cr0, eax

    ; Far Jump in 32-Bit-Code (pm_entry)
    jmp 0x08:pm_entry      ; 0x08 = Code-Segment-Selector
    
    
load_error:
    mov si, msgB2Fail
    call print_string

    ; Debug: Fehlercode ausgeben
    mov al, ah             ; AH = BIOS Error Code
    call print_hex8        ; z.B. " AH=0E"

.halt:
    cli
    hlt
    jmp .halt

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
    push si
    push di
    push ax
    push bx

    xor ax, ax
    mov ds, ax              ; DS=0000
    mov ax, 0xFFFF
    mov es, ax              ; ES=FFFF

    mov di, 0x0500          ; phys: 0000:0500 = 0x00500
    mov si, 0x0510          ; phys: FFFF:0510 = 0x100500 (A20 entscheidet!)

    mov al, [ds:di]
    mov bl, [es:si]
    push ax
    push bx

    mov byte [ds:di], 0x00
    mov byte [es:si], 0xFF

    cmp byte [ds:di], 0xFF  ; wenn A20 AUS, landet 0xFF auch bei 0x00500
    ; ZF=1 => A20 AUS
    ; ZF=0 => A20 AN

    pop bx
    pop ax
    mov [es:si], bl
    mov [ds:di], al

    pop bx
    pop ax
    pop di
    pop si
    pop es
    pop ds
    popf
    ret
    
; -------------------------------------------------
; VESA Mode 0x114 mit LFB setzen und Infos sichern
; -------------------------------------------------
set_vesa_mode_800x600:
    ; 1) Mode-Info holen
    mov ax, 0x4F01   ; VBE-Funktion: Get Mode Info
    mov cx, 0x0114   ; gewünschter Modus: 0x114
    xor bx, bx
    mov es, bx
    mov di, 0x2000  ; ES -> 0x2000 -> ES:DI = 0000:2000 (phys 0x00002000)
    int 0x10
    
    cmp ax, 0x004F
    jne .fail_setmode
    
    mov ax, 0x4F02
    mov bx, 0x4114
    int 0x10
    
    cmp ax, 0x004F
    jne .fail_setmode

    ; -------------------------------------------------
    ; GDT laden, Protected Mode aktivieren
    ; -------------------------------------------------
    cli
    ;lgdt [gdt_descriptor]
    
    mov eax, cr0
    or  eax, 1             ; PE-Bit setzen
    mov cr0, eax
    
    ; Far Jump in 32-Bit-Code (pm_entry)
    jmp 0x08:pm_entry      ; 0x08 = Code-Segment-Selector

    ret

.fail_controller:
    mov si, msgVESA_failController
    call print_string
    jmp .failVESA
.fail_modeinfo:
    mov si, msgVESA_failModeInfo
    call print_string
    jmp .failVESA
.fail_unsupported:
    mov si, msgVESA_failUnsupported
    call print_string
    jmp .failVESA
.fail_setmode:
    mov ax, 3
    int 0x10
    mov si, msgVESA_failSetmode
    call print_string
.failVESA:
    pop es
    pop ds
    popad
    stc
    ret

;---------------------------------------------------------
; Daten
;---------------------------------------------------------
BITS 16
boot_drive      db 0

kernel_lba_low  dd KERNEL_LBA
kernel_lba_high dd 0

sectors_left    dw 0
sectors_done    dw 0

msgStage2Error  db "[Stage2] Error ", 0
msgB2Start      db 13,10,"[Stage2] BOOT2 gestartet, Kernel wird geladen -> 4MB ...",13,10,0
msgB2OK         db "[Stage2] Kernel geladen, springe nach 8000:0000",13,10,0
msgA20OK        db "[Stage2] A20 aktiviert, Protected Mode wird gestartet ...",13,10,0
msgA20FAIL      db "[Stage2] A20 Fehler",13,10,0
msgB2Fail       db "[Stage2] FEHLER beim Laden des Kernels",13,10,0
msgBeforePM     db "[Stage2] Protected Mode CLI",13,10,0
msgVESAerr      db "[Stage2] VESA Modus Fehler!",13,10,0

disk_status_error_00 db ": 0x00 successful completion.", 0
disk_status_error_01 db ": 0x01 invalid function in AH or invalid parameter.", 0
disk_status_error_02 db ": 0x02 address mark not found.", 0
disk_status_error_03 db ": 0x03 disk write-protected.", 0
disk_status_error_04 db ": 0x04 sector not found/read error.", 0
disk_status_error_05 db ": 0x05 reset failed.", 0
disk_status_error_06 db ": 0x06 disk changed.", 0
disk_status_error_07 db ": 0x07 drive parameter activity failed.", 0
disk_status_error_08 db ": 0x08 DMA overrun.", 0
disk_status_error_09 db ": 0x09 data boundary error (attempted DMA across 64K boundary or >80h sectors).", 0
disk_status_error_0A db ": 0x0A bad sector detected.", 0
disk_status_error_0B db ": 0x0B bad track detected.", 0
disk_status_error_0C db ": 0x0C unsupported track or invalid media.", 0
disk_status_error_0D db ": 0x0D invalid number of sectors on format (PS/2 hard disk).", 0
disk_status_error_0E db ": 0x0E control data address mark detected.", 0
disk_status_error_0F db ": 0x0F DMA arbitration level out of range.", 0
disk_status_error_10 db ": 0x10 uncorrectable CRC or ECC error on read.", 0
disk_status_error_11 db ": 0x11 data ECC corrected.", 0
disk_status_error_20 db ": 0x20 controller failure.", 0
disk_status_error_31 db ": 0x31 no media in drive (IBM/MS INT 13 extensions).", 0
disk_status_error_32 db ": 0x32 incorrect drive type stored in CMOS (Compaq).", 0
disk_status_error_40 db ": 0x40 seek failed.", 0
disk_status_error_80 db ": 0x80 timeout (not ready).", 0
disk_status_error_AA db ": 0xAA drive not ready (hard disk)", 0
disk_status_error_B0 db ": 0xB0 volume not locked in drive.", 0
disk_status_error_B1 db ": 0xB1 volume locked in drive.", 0
disk_status_error_B2 db ": 0xB2 volume not removable.", 0
disk_status_error_B3 db ": 0xB3 volume in use", 0
disk_status_error_B4 db ": 0xB4 lock count exceeded.", 0
disk_status_error_B5 db ": 0xB5 valid eject request failed.", 0
disk_status_error_B6 db ": 0xB6 volume present but read protected.", 0
disk_status_error_BB db ": 0xBB undefined error.", 0
disk_status_error_CC db ": 0xCC write fault.", 0
disk_status_error_E0 db ": 0xE0 status register error.", 0
disk_status_error_FF db ": 0xFF sense operation failed.", 0

disk_status_error_table:
  db 0x00
  dw disk_status_error_00
  db 0x01
  dw disk_status_error_01
  db 0x02
  dw disk_status_error_02
  db 0x03
  dw disk_status_error_03
  db 0x04
  dw disk_status_error_04
  db 0x05
  dw disk_status_error_05
  db 0x06
  dw disk_status_error_06
  db 0x07
  dw disk_status_error_07
  db 0x08
  dw disk_status_error_08
  db 0x09
  dw disk_status_error_09
  db 0x0A
  dw disk_status_error_0A
  db 0x0B
  dw disk_status_error_0B
  db 0x0C
  dw disk_status_error_0C
  db 0x0D
  dw disk_status_error_0D
  db 0x0E
  dw disk_status_error_0E
  db 0x0F
  dw disk_status_error_0F
  db 0x10
  dw disk_status_error_10
  db 0x11
  dw disk_status_error_11
  db 0x20
  dw disk_status_error_20
  db 0x31
  dw disk_status_error_31
  db 0x32
  dw disk_status_error_32
  db 0x40
  dw disk_status_error_40
  db 0x80
  dw disk_status_error_80
  db 0xAA
  dw disk_status_error_AA
  db 0xB0
  dw disk_status_error_B0
  db 0xB1
  dw disk_status_error_B1
  db 0xB2
  dw disk_status_error_B2
  db 0xB3
  dw disk_status_error_B3
  db 0xB4
  dw disk_status_error_B4
  db 0xB5
  dw disk_status_error_B5
  db 0xB6
  dw disk_status_error_B6
  db 0xBB
  dw disk_status_error_BB
  db 0xCC
  dw disk_status_error_CC
  db 0xE0
  dw disk_status_error_E0
  db 0xFF
  dw disk_status_error_FF

msgAH      db " AH=",0

; einfache 3-Entry-GDT: null, code, data
gdt_start:
gdt_null:   dq 0
gdt_code:   dq 0x00CF9A000000FFFF   ; base=0, limit=4GB, Code, RX
gdt_data:   dq 0x00CF92000000FFFF   ; base=0, limit=4GB, Data, RW
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEL equ (gdt_code - gdt_start)
DATA_SEL equ (gdt_data - gdt_start)

;---------------------------------------------------------
; Disk Address Packet
;---------------------------------------------------------
disk_address_packet:
dap_size        db 0x10
dap_reserved    db 0x00
dap_sectors     dw 0
dap_offset      dw 0
dap_segment     dw 0
dap_lba_low     dd 0
dap_lba_high    dd 0

dotstr1 db '1',0
dotstr2 db '2',0
dotstr3 db '3',0
dotstr4 db '4',0
dotstr5 db '5',0

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

msgVESA_failController:     db "[VESA] Error: Controller." , 0
msgVESA_failModeInfo:       db "[VESA] Error: ModeInfo."   , 0
msgVESA_failUnsupported:    db "[VESA] Error: Unsupported.", 0
msgVESA_failSetmode:        db "[VESA] Error: SetMode."    , 0

vessatext_a: db "vESSA 1111", 13, 10, 0
vessatext_b: db "vESSA CCCC", 13, 10, 0
vessatext_c: db "gugu ",      13, 10, 0
menuFlag db 4

msg_loaded  db "boot2: kernel loaded, jump PM...", 13, 10, 0
msg_diskerr db "boot2: disk read error!", 13, 10, 0

disk_error_ext db "boot2: no disk extensions.",13,0

loaddd: db "lllooo ", 0
; Puffer für VBE Mode Info (256 Byte reichen)
vesa_mode_info:
    times 256 db 0
