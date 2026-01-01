; ---------------------------------------------------------------------------
; \file  boot2.asm
; \note  (c) 2025 by Jens Kallup - paule32
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
    jc load_error

    mov si, msgB2OK
    call print_string

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
    hlt
    
    A20ok:
    sti
    call get_vesa_mode
;   call set_vesa_mode

    ; -------------------------------------------------
    ; GDT laden, Protected Mode aktivieren
    ; -------------------------------------------------
    mov si, msgBeforePM
    call print_string
    
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or  eax, 1             ; PE-Bit setzen
    mov cr0, eax

    ; Far Jump in 32-Bit-Code (pm_entry)
    jmp 0x08:pm_entry      ; 0x08 = Code-Segment-Selector
    
    
    ; -------------------------------------------------
    ; In den Kernel springen
    ; -------------------------------------------------
    ;mov dl, [boot_drive]      ; Laufwerk an Kernel weiterreichen (falls gebraucht)
    ;jmp 0x1000:0000           ; CS:IP = 1000:0000

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

BITS 32
pm_entry:
    ; Segmente im Protected Mode setzen
    mov ax, 0x10          ; Data-Segment-Selector
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    mov dword [0xB8000], 0x07420741  ; "AB"
    mov esp, 0x0009F000   ; irgendein 32-Bit-Stack im oberen Bereich
    
    mov dword [0xB8000], 0x072A072A  ; "**" (2 Zeichen) weiß auf schwarz
    
    ; entrypoint steht als dword am Anfang des
    ; geladenen kernel.bin (kernel.ld)
    mov eax, [0x00080000]
    jmp eax

    ; Jetzt zum Kernel-Einstieg springen (0x00080000)
    ;jmp 0x00080000        ; KernelStart wurde auf 0x00080000 gelinkt

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

; Puffer für VBE Mode Info (256 Byte reichen)
vesa_mode_info:
    times 256 db 0
