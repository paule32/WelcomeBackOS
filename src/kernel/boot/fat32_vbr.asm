; fat32_vbr_min.asm - tiny VBR that loads raw stage2 from fixed LBAs
; Loads STAGE2_SECTORS sectors from (PART_START_LBA + 1) to 0x7C00 and jumps.
; 512 bytes total.

BITS 16
ORG 0x7C00

%ifndef STAGE2_SECTORS
%define STAGE2_SECTORS 32
%endif

start:
    jmp short entry
    nop

; --- 90 bytes BPB area placeholder (we patch this from mformat) ---
times 90-($-$$) db 0

entry:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    ; INT13 extensions?
    mov ax, 0x4100
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 0x13
    jc  fail
    cmp bx, 0xAA55
    jne fail
    test cx, 1
    jz  fail

    ; Read STAGE2_SECTORS from LBA = (partition start + 1) into 0000:7C00
    mov word [dap+2], STAGE2_SECTORS
    mov word [dap+4], 0x7C00
    mov word [dap+6], 0x0000

    mov eax, [part_lba]     ; partition LBA start (patched by MBR before jump OR filled by MBR)
    inc eax                 ; +1 => after VBR sector
    mov dword [dap+8], eax
    mov dword [dap+12], 0

    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc  fail

    jmp 0x0000:0x7C00

fail:
    mov si, msg
.p:
    lodsb
    test al, al
    jz .h
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .p
.h:
    cli
    hlt
    jmp .h

boot_drive db 0

; partition start LBA (will be set by the MBR before jumping here)
part_lba   dd 0

dap:
    db 0x10, 0x00
    dw 0, 0, 0
    dq 0

msg db "VBR: load stage2 failed", 0

times 510-($-$$) db 0
dw 0xAA55
