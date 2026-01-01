; mbr_usb.asm - Minimal MBR that loads boot2 from fixed LBA and jumps to it
; BIOS only, 16-bit real mode
; Assembled to exactly 512 bytes

BITS 16
ORG 0x7C00

%ifndef STAGE2_LBA
%define STAGE2_LBA 1
%endif

%ifndef STAGE2_SECTORS
%define STAGE2_SECTORS 32
%endif

%ifndef STAGE2_LOAD
%define STAGE2_LOAD 0x8000
%endif

%ifndef KERNEL_LBA
%define KERNEL_LBA (STAGE2_LBA + STAGE2_SECTORS)
%endif

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    ; Check INT13 extensions present (AH=41h)
    mov ax, 0x4100
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 0x13
    jc  disk_error
    cmp bx, 0xAA55
    jne disk_error
    test cx, 1
    jz  disk_error

    ; Build Disk Address Packet (DAP) for AH=42h (extended read)
    mov word [dap+2], STAGE2_SECTORS      ; number of sectors
    mov word [dap+4], STAGE2_LOAD        ; offset
    mov word [dap+6], 0x0000             ; segment
    mov dword [dap+8], STAGE2_LBA        ; LBA low (32-bit is fine here)
    mov dword [dap+12], 0                ; LBA high

    ; Read stage2
    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc  disk_error

    ; Pass info to stage2 (simple convention)
    ; DL = boot drive
    ; EAX = kernel LBA
    ; EBX = kernel sector count (optional: stage2 can compute/ignore)
    mov dl, [boot_drive]
    db 0x66
    mov ax, KERNEL_LBA
    db 0x66
    xor bx, bx

    jmp 0x0000:STAGE2_LOAD

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

boot_drive db 0

msg db "USB MBR: disk read error", 0

; -------------------------
; Disk Address Packet (16 bytes)
; -------------------------
dap:
    db 0x10        ; size
    db 0x00        ; reserved
    dw 0           ; sectors (patched)
    dw 0           ; offset  (patched)
    dw 0           ; segment (patched)
    dq 0           ; LBA (patched)

; -------------------------
; Partition table (optional but helps some BIOSes)
; One "dummy" partition marked active starting at LBA 1
; -------------------------
times 446-($-$$) db 0

pt1:
    db 0x80        ; bootable
    db 0x00,0x02,0x00
    db 0x83        ; type (Linux) - dummy
    db 0x00,0xFE,0xFF
    dd 1           ; start LBA
    dd 0x00010000  ; size in sectors (dummy large)
times 16*3 db 0

dw 0xAA55
