; ---------------------------------------------------------------------------
; \file  boot1.asm
; \note  (c) 2025 by Jens Kallup - paule32
;        all rights reserved.
; ---------------------------------------------------------------------------
%include 'lba.inc'

BITS 16
ORG 0x7C00

start:
    jmp near entry
    nop
entry:
    cli
    mov ax, 0x0000   ; Segment auf 0x0000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x8000
    sti
    
    ; Boot-Laufwerk aus DL merken
    mov [boot_drive], dl

    ; --- Schritt 1: EDD (INT 13h Extensions) testen ---
    mov si, msgHello
    call print_string
    
    mov si, msgCheckEDD
    call print_string

    mov ax, 0x4100         ; AH=41h, „Installation Check“
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 0x13

    jc  no_edd             ; CF=1 -> keine Extensions
    cmp bx, 0xAA55
    jne no_edd            ; Magic falsch -> keine Extensions

    ; Extensions vorhanden, AH enthält Versionsinfo
    mov si, msgEDDYes
    call print_string
    jmp try_lba

no_edd:
    mov si, msgEDDNo
    call print_string
    jmp $                ; ohne EDD bringt AH=42 eh nix

try_lba:
    ; --- Schritt 2: LBA-Read testen (AH=42h) ---
    mov word  [dap_sectors ], 1          ; 1 Sektor
    mov word  [dap_offset  ], 0x0600     ; Offset
    mov word  [dap_segment ], 0x0000     ; Segment
    mov dword [dap_lba_low ], TEST_LBA
    mov dword [dap_lba_high], 0

    mov si, disk_address_packet
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jnc lba_ok

    ; CF=1 -> Fehler, AH = Statuscode
    mov si, msgLBABad
    call print_string

    mov al, ah            ; Fehlercode in AL
    call print_hex8       ; "AH=xx" anzeigen

    jmp $

lba_ok:
    mov si, msgLBAOk
    call print_string
    
    ;---------------------------------------------------------
    mov si, msgLoad2
    call print_string

    ; -------------------------------------------------
    ; Boot2 per LBA laden
    ; -------------------------------------------------
    mov word  [dap_sectors ], BOOT2_SECTORS
    mov word  [dap_offset  ], 0x0500
    mov word  [dap_segment ], 0x0000
    mov dword [dap_lba_low ], BOOT2_LBA
    mov dword [dap_lba_high], 0

    mov si, disk_address_packet
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc load_error

    mov si, msgOK
    call print_string
    
    ; -------------------------------------------------
    ; zu BOOT2 springen → CS:IP = 0000:0500
    ; -------------------------------------------------
    jmp 0x0000:0x0500

load_error:
    mov si, msgErr
    call print_string
    jmp $

;---------------------------------------------------------
; print_string: DS:SI -> 0-terminierte Zeichenkette
;---------------------------------------------------------
print_string:
    lodsb
    cmp al, 0
    je .ps_done
    cmp dx, 1
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

;---------------------------------------------------------
; Daten
;---------------------------------------------------------
boot_drive      db 0

msgHello        db "Stage1: CD boot debug",13,10,0
msgCheckEDD     db "Checking INT13h extensions ...",13,10,0
msgEDDYes       db "EDD present.",13,10,0
msgEDDNo        db "EDD NOT present.",13,10,0

msgLBABad       db "LBA read FAILED,",0
msgLBAOk        db "LBA read OK.",13,10,0
msgAH           db " AH=",0

msgLoad2        db "Loading BOOT2.SYS ...",13,10,0
msgOK           db "BOOT2 loaded",13,10,0
msgErr          db "ERROR loading BOOT2",13,10,0

;---------------------------------------------------------
; Disk Address Packet (16 Byte)
;---------------------------------------------------------
disk_address_packet:
dap_size      db 0x10      ; Groesse des Pakets
dap_reserved  db 0
dap_sectors   dw 0         ; Sektoren
dap_offset    dw 0         ; Offset
dap_segment   dw 0         ; Segment
dap_lba_low   dd 0         ; Start-LBA low
dap_lba_high  dd 0         ; Start-LBA high

; Bootsignature
times 510-($-$$) db 0
dw 0xAA55
