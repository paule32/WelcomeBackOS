; ---------------------------------------------------------------------------
; mbr_fat.asm
; Minimal BIOS MBR for FAT32 USB/HDD boot
;
; - finds active partition
; - reads its VBR (1 sector) to 0x7C00
; - patches partition start LBA into VBR at offset 0x5C
; - jumps to VBR
;
; Works with fat32_vbr_min.asm
; ---------------------------------------------------------------------------

BITS 16
ORG 0x7C00

%define VBR_LOAD_ADDR   0x7C00
%define PART_LBA_OFF    0x5C      ; must match VBR layout!

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    ; -------------------------------------------------------
    ; find active partition (status 0x80)
    ; -------------------------------------------------------
    mov si, 0x7C00 + 446      ; partition table
    mov cx, 4

.find_active:
    cmp byte [si], 0x80
    je  .found
    add si, 16
    loop .find_active
    jmp disk_error

.found:
    ; read partition start LBA
    mov eax, [si + 8]
    mov [part_lba], eax

    ; -------------------------------------------------------
    ; check INT13 extensions
    ; -------------------------------------------------------
    mov ax, 0x4100
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 0x13
    jc  disk_error
    cmp bx, 0xAA55
    jne disk_error
    test cx, 1
    jz  disk_error

    ; -------------------------------------------------------
    ; read VBR (1 sector) to 0000:7C00
    ; -------------------------------------------------------
    mov word [dap + 2], 1              ; sectors
    mov word [dap + 4], VBR_LOAD_ADDR  ; offset
    mov word [dap + 6], 0x0000         ; segment
    mov eax, [part_lba]
    mov dword [dap + 8], eax           ; LBA low
    mov dword [dap + 12], 0            ; LBA high

    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc  disk_error

    ; -------------------------------------------------------
    ; patch partition LBA into VBR (runtime data)
    ; VBR expects it at offset 0x5C
    ; -------------------------------------------------------
    mov eax, [part_lba]
    mov di, VBR_LOAD_ADDR + PART_LBA_OFF
    mov [di], eax

    ; -------------------------------------------------------
    ; jump to VBR
    ; -------------------------------------------------------
    jmp 0x0000:VBR_LOAD_ADDR

; -------------------------------------------------------
; error handling
; -------------------------------------------------------
disk_error:
    mov si, msg
.print:
    lodsb
    test al, al
    jz .hang
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .print

.hang:
    cli
    hlt
    jmp .hang

; -------------------------------------------------------
; data
; -------------------------------------------------------
boot_drive db 0
part_lba   dd 0

msg db "MBR: boot failed", 0

; -------------------------------------------------------
; Disk Address Packet (INT 13h AH=42h)
; -------------------------------------------------------
dap:
    db 0x10        ; size
    db 0x00
    dw 0           ; sectors
    dw 0           ; offset
    dw 0           ; segment
    dq 0           ; LBA

; -------------------------------------------------------
; padding up to partition table
; -------------------------------------------------------
times 446-($-$$) db 0

; partition table (written by image builder)
times 16*4 db 0

; -------------------------------------------------------
; boot signature
; -------------------------------------------------------
dw 0xAA55
