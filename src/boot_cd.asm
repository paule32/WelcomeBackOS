; boot_cd.asm - El Torito no-emulation Bootsector für CD
BITS 16
ORG 0x7C00

jmp short entry_point
nop

; Optional könntest du hier noch ein paar Dummy-BPB-Felder lassen
; damit Tools nicht meckern, ist aber für CD nicht zwingend nötig.

;-------------------------------------------------------------------------------
; Einstiegspunkt
;-------------------------------------------------------------------------------
entry_point:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ax, 0x7C00
    mov ss, ax
    xor sp, sp

    ; BIOS übergibt Boot-Laufwerk in DL
    mov [bootdevice], dl

    mov si, msgLoading
    call print_string

    ; DAP für Stage2 vorbereiten
    ; Ziel: 0000:0500 (wie in deinem bisherigen Loader)
    mov word [dap_sectors], STAGE2_SECTORS
    mov word [dap_offset],  0x0500
    mov word [dap_segment], 0x0000
    mov dword [dap_lba_low],  STAGE2_LBA
    mov dword [dap_lba_high], 0

    ; LBA lesen
    mov si, disk_address_packet
    call read_lba
    jc disk_error

    mov si, msgCRLF
    call print_string

    ; zu Stage2 / Kernel springen (far return, wie bei dir)
    mov dl, [bootdevice]
    push word 0x0000       ; CS
    push word 0x0500       ; IP
    retf

disk_error:
    mov si, msgFailure
    call print_string
    cli
    hlt
    jmp $

;-------------------------------------------------------------------------------
; read_lba: nutzt INT 13h, AH=42h (EDD)
; DS:SI -> Disk Address Packet
;-------------------------------------------------------------------------------
read_lba:
    mov dl, [bootdevice]   ; Bootdrive in DL
    mov ah, 0x42
    int 0x13
    ret

;-------------------------------------------------------------------------------
; print_string: DS:SI -> 0-terminierte Zeichenkette
;-------------------------------------------------------------------------------
print_string:
    mov cx, 0
.next:
    lodsb
    cmp al, 0
    je .done
    cmp cx, 50        ; wie bei dir – max. 50 Zeichen
    je .done
    mov ah, 0x0E
    mov bh, 0
    mov bl, 0x07
    int 0x10
    inc cx
    jmp .next
.done:
    ret

;-------------------------------------------------------------------------------
; Daten
;-------------------------------------------------------------------------------
bootdevice  db 0

msgCRLF     db 13,10,0
msgLoading  db "Loading stage2 from CD ...",13,10,0
msgFailure  db 13,10,"CD LOAD ERROR",13,10,0

;-------------------------------------------------------------------------------
; Disk Address Packet (16 Byte)
;-------------------------------------------------------------------------------
disk_address_packet:
dap_size      db 0x10      ; Größe des Pakets
dap_reserved  db 0x00
dap_sectors   dw 0         ; Anzahl Sektoren
dap_offset    dw 0         ; Offset
dap_segment   dw 0         ; Segment
dap_lba_low   dd 0         ; Start-LBA (low dword)
dap_lba_high  dd 0         ; Start-LBA (high dword, meist 0)

;-------------------------------------------------------------------------------
; Konstanten: musst du von außen setzen/patchen
;-------------------------------------------------------------------------------
STAGE2_LBA      equ 50     ; Beispiel: ab LBA 50
STAGE2_SECTORS  equ 200    ; Beispiel: 200 Sektoren à 512 Byte -> 100 KiB

;-------------------------------------------------------------------------------
; Boot-Signatur
;-------------------------------------------------------------------------------
times 510-($-$$) db 0
dw 0xAA55
