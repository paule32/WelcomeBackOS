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
    mov  word [ExecBlk + 0x00], 0
    mov  word [ExecBlk + 0x02], GzipCmdTail  ; OFFSET
    mov  word [ExecBlk + 0x04], ds           ; SEGMENT
    mov  word [ExecBlk + 0x06], 0x005C       ; FCB1 OFFSET
    mov  word [ExecBlk + 0x08], cs           ; FCB1 SEGMENT
    mov  word [ExecBlk + 0x0A], 0x006C       ; FCB2 OFFSET
    mov  word [ExecBlk + 0x0C], cs           ; FCB2 SEGMENT

    ; -------------------------------------------------------
    ; EXEC AH=4Bh, AL=00h (Load & Execute) ...
    ; -------------------------------------------------------
    mov  dx, GzipAppName    ; ASCIIZ "gzip.exe"
    push ds
    pop  es
    mov  bx, ExecBlk        ; Parameterblock
    mov  ax, 0x4B00
    int  0x21
    jc   go_error           ; CF=1 => Fehler, AX=Errorcode

    ; -------------------------------------------------------
    ; optional: return code of child (INT 21h/4Dh)
    ; -------------------------------------------------------
    mov  ax, 0x4D00
    int  0x21
    jc   go_error
    jnc  go_forward
    
    go_error:
    mov ax, 0x4C00
    mov bx, 1
    int 0x21
    
    go_forward:
    mov ax, 0x4C00
    mov bx, 0
    int 0x21

    ; initialize segments
    cli
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xFFFE       ; set simple stack
    sti

    ; -------------------------------
    ; open KERNEL.BIN
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

; ---------------------------------------------------------------------------
; GDT ...
; ---------------------------------------------------------------------------
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
kernel_fname db "KERNEL.BIN",0

msg_file db "open error: KERNEL.BIN$",0
msg_read db "read error: KERNEL.BIN$",0

GzipAppName:    db 'GZ.EXE', 0
GzipErrorMsg:   db 'exec of gzip.exe failed.', 0

GzipCmdTail:
    db GzipCmdTextEnd - GzipCmdText          ; Länge
GzipCmdText:
    db ' -d KERNEL.BIZ'
GzipCmdTextEnd:
    db 13

section .bss
; EXEC-Parameterblock (14 Bytes)
ExecBlk:        resb 14
