; ---------------------------------------------------------------------------
; \file loader.asm – A simple freestanding C-Kernel
; \note  (c) 2025 by Jens Kallup - paule32
;        all rights reserved.
; ---------------------------------------------------------------------------
; loader.asm – minimal 16-Bit-DOS-Loader, that load KERNEL.BIN.
; Build: nasm -f bin loader.asm -o LOADER.COM
; ---------------------------------------------------------------------------
org  0x100               ; .COM start offset
bits 16

start:
    mov dx, msg_before
    mov ah, 9
    int 21h
    
    mov ax, cs
    mov ds, ax
    mov es, ax
    
    mov word [exec_block_env_seg], 0
    
    mov [exec_block_cmd_seg ], ax
    mov [exec_block_fcb1_seg], ax
    mov [exec_block_fcb2_seg], ax
    
    mov word [exec_block_env_seg_2], 0
    
    mov [exec_block_cmd_seg_2 ], ax
    mov [exec_block_fcb1_seg_2], ax
    mov [exec_block_fcb2_seg_2], ax

    ; -------------------------------
    ; open KERNEL.BIZ
    ; -------------------------------
    mov dx, kernel_fgzip
    mov ax, 0x4300
    int 0x21
    jnc depacked
    
    cmp ax, 0x02
    je  __check_02
    cmp ax, 0x03
    je  error_03
    cmp ax, 0x05
    je  error_05
    
    jmp packed
    
    depacked:
    mov dx, GzipAppName
    mov bx, exec_block_2
    mov ax, 0x4B00           ; 4B00h = load & execute
    int 0x21
    jc exec_error
    
    __check_02:
    packed:
    mov dx, GzipAppName
    mov bx, exec_block
    mov ax, 0x4B00           ; 4B00h = load & execute
    int 0x21
    jc exec_error
    
    mov dx, msg_after
    mov ah, 9
    int 0x21
    
    jmp exit_ok

    exec_error:
    cmp ax, 0x02
    je  error_02
    cmp ax, 0x03
    je  error_03
    cmp ax, 0x08
    je  error_08

    error_02:
    mov dx, msg_error_02
    mov ah, 9
    int 0x21
    jmp exit_error
    error_03:
    mov dx, msg_error_03
    mov ah, 9
    int 0x21
    jmp exit_error
    error_08:
    mov dx, msg_error_05
    mov ah, 9
    int 0x21
    jmp exit_error
    error_05:
    mov dx, msg_error_08
    mov ah, 9
    int 0x21
    jmp exit_error

    exit_error:
    mov dx, msg_error
    mov ah, 9
    int 0x21
    
    mov ax, 0x4C01
    int 0x21
    
    exit_ok:
    mov dx, GzipAppName
    mov bx, exec_block_2
    mov ax, 0x4B00           ; 4B00h = load & execute
    int 0x21
    jc exec_error

    ; initialize segments
    cli
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xFFFE       ; set simple stack
    sti

    ; -------------------------------
    ; open KERNEL.BI
    ; -------------------------------
    mov dx, kernel_fname ; DS:DX -> ASCIIZ-filebame
    mov ax, 0x3D00       ; DOS: open (read-only)
    int 0x21
    jc file_error        ; open error?
    mov bx, ax           ; file handle

    ; target segment, kernel load: 0x1000 -> 0x00010000
    xor dx, dx           ; offset

read_loop:
    push ds
    mov ax, 0x4000
    mov ds, ax
    mov ah, 0x3F         ; DOS: read
    mov cx, 512          ; up to 512 bytes per read
    int 0x21
    pop ds
    jc read_error
    or ax, ax
    jz done_read         ; 0 bytes -> eof
    add dx, ax           ; increment offset
    jmp read_loop

done_read:
    ; close file
    mov ah, 0x3E
    int 0x21

    ; --------------------------------
    ; pre-pare Protected Mode
    ; activate A20 -> TODO !
    ; --------------------------------
    cli

    mov dx, ds
    movzx edx, dx
    shl edx, 4
    add [gdt_descriptor + 2], edx

    ; load GDT
    lgdt [gdt_descriptor]

    ; set PE-Bit at CR0
    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    ; Far Jump in 32-Bit Code: Selector 0x08 (Code-Segment)
    push 0x08
    add edx, protected_mode_entry
    push edx
    jmp far dword [esp]

; ---------------------------------------------------------------------------
; Erorr Handling
; ---------------------------------------------------------------------------
file_error:
    mov dx, msg_file
    mov ah, 0x09
    int 0x21
    jmp exit_dos

read_error:
    mov dx, msg_read
    mov ah, 0x09
    int 0x21
    jmp exit_dos

exit_dos:
    mov ax, 0x4C01       ; DOS: terminate with error code 1
    int 0x21

    mov ax, 0x4C00
    mov bx, 0
    int 0x21

; ---------------------------------------------------------------------------
; GDT ...
; ---------------------------------------------------------------------------
section .data
align 8
gdt_start:
    dq 0x0000000000000000       ; Null-Deskriptor
    dq 0x00CF9A000000FFFF       ; Code: Base=0, Limit=4GB, 32-Bit
    dq 0x00CF92000000FFFF       ; Data: Base=0, Limit=4GB, 32-Bit
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; ---------------------------------------------------------------------------
; 32-Bit Code ...
; ---------------------------------------------------------------------------
bits 32
protected_mode_entry:
    ; Segmente setzen (Selector 0x10 = Data-Segment)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; simple stack somewhere in Low-Memory
    mov esp, 0x0090000

    ; Kernel-Entrypoint (Address from linker script!)
    call [0x40000] ; physiscall Adress of KERNEL.BIN-Entry: 0x1000:0000

hang:
    jmp hang

; ---------------------------------------------------------------------------
; Data (16-Bit)
; ---------------------------------------------------------------------------
bits 16
kernel_fname db "KERNEL.BIZ",0
kernel_fgzip db "KERNEL.BI",0

msg_file db "open error: KERNEL.BI$",0
msg_read db "read error: KERNEL.BI$",0

GzipAppName:    db 'GZ.EXE', 0
GzipErrorMsg:   db 'exec of gzip.exe failed.', 0

exec_block:
exec_block_env_seg   dw 0

exec_block_cmd_off   dw GzipCmdTail
exec_block_cmd_seg   dw 0

exec_block_fcb1_off  dw 5Ch
exec_block_fcb1_seg  dw 0

exec_block_fcb2_off  dw 6Ch
exec_block_fcb2_seg  dw 0

exec_block_2:
exec_block_env_seg_2   dw 0

exec_block_cmd_off_2   dw GzipCmdTail_2
exec_block_cmd_seg_2   dw 0

exec_block_fcb1_off_2  dw 5Ch
exec_block_fcb1_seg_2  dw 0

exec_block_fcb2_off_2  dw 6Ch
exec_block_fcb2_seg_2  dw 0

GzipCmdTail:
    db GzipCmdTextEnd - GzipCmdText          ; Länge
GzipCmdText:
    db ' -d KERNEL.BIZ'
GzipCmdTextEnd:
    db 13

GzipCmdTail_2:
    db GzipCmdTextEnd_2 - GzipCmdText_2          ; Länge
GzipCmdText_2:
    db ' -9 KERNEL.BI'
GzipCmdTextEnd_2:
    db 13

msg_before db "start GZ.EXE ...", 13, 10, "$"
msg_after  db 13,10, "GZ.EXE return.", 13,10, "$"
msg_error  db 13,10, "Error: GZ.EXE could not be load.", 13,10, "$"

msg_error_02: db "Error: file not found.", 13, 10, '$'
msg_error_03: db "Error: path not found.", 13, 10, '$'
msg_error_05: db "Error: access denied." , 13, 10, '$'
msg_error_08: db "Error: memory error."  , 13, 10, '$'

section .bss
; EXEC-Parameterblock (14 Bytes)
ExecBlk:        resb 14
