; ---------------------------------------------------------------------------
; \file loader.asm – A simple freestanding C-Kernel
; \note  (c) 2025 by Jens Kallup - paule32
;        all rights reserved.
; ---------------------------------------------------------------------------
; loader.asm – Minimaler 16-Bit-DOS-Loader, der KERNEL.BIN lädt
; Build: nasm -f bin loader.asm -o LOADER.COM
; ---------------------------------------------------------------------------
Bits 16
org 0x500
jmp entry_point             ; go to entry point

;*******************************************************
;	Includes and Defines
;*******************************************************
%include "gdt.inc"			; GDT definition
%include "A20.inc"			; A20 gate enabling
%include "Fat12.inc"		; FAT12 driver
%include "GetMemoryMap.inc" ; INT 0x15, eax = 0xE820 

%define IMAGE_PMODE_BASE 0x40000 ; where the kernel is to be loaded to in protected mode
%define IMAGE_RMODE_BASE 0x3000  ; where the kernel is to be loaded to in real mode
ImageName     db "KERNEL  SYS"
ImageSize     dw 0

;*******************************************************
;	Data Section
;*******************************************************
msgLoading db 13, 10, "jump to OS Kernel...", 0
msgFailure db 13, 10, "missing KERNEL.SYS"  , 13, 0
msgNoVBE   db 13, 10, "no VBE gfx support." , 13, 0

no_vbe:
    mov si, msgNoVBE
    call    print_string
    .loop:
    jmp .loop
    
entry_point:
    cli	                 ; clear interrupts
    xor ax, ax           ; null segments
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xFFFF       ; stack begins at 0xffff (downwards)
    sti	                 ; enable interrupts
    
;    push es
    xor  ax, ax
    mov  ds, ax          ; DS = 0
    mov  ax, 0x9000
    mov  es, ax          ; ES = 0x9000
    mov  di, 0x0000      ; ES:DI = 9000:0000 -> phys 0x00090000
                         ; see stack: 0x9FC00 in protected mode !
                         
	mov  ax, 0x4F01      ; get VBE mode info
    mov  cx, 0x114       ; 800x600x16bpp mode

	int  0x10
;	pop  es
    
    cmp  ax, 0x004F      ; test for error
    jne  no_vbe
;jmp A20
;    push es
    mov  ax, 0x4f02
    mov  bx, 0x4114       ; enable LFB
    int  0x10
;    pop  es
    
A20:	
   call EnableA20

;*******************************************************
;   Determine physical memory INT 0x15, eax = 0xE820                     
;   input: es:di -> destination buffer         
;*******************************************************
Get_Memory_Map:
    xor eax, eax
    mov ds, ax
    mov di, 0x1000
    call get_memory_by_int15_e820
    xor ax, ax
    mov es, ax ; important to null es!

Install_GDT:
    call InstallGDT
    sti	
	
Load_Root:
    call LoadRoot
    mov ebx, 0
    mov ebp, IMAGE_RMODE_BASE
    mov esi, ImageName
    call LoadFile
    mov DWORD [ImageSize], ecx
    cmp ax, 0
    je EnterProtectedMode
    mov si, msgFailure
    call print_string
    xor ah, ah

;*******************************************************
;   Switch from Real Mode (RM) to Protected Mode (PM)              
;*******************************************************
EnterProtectedMode:
    ; switch off floppy disk motor
    mov dx,0x3F2      
    mov al,0x0C
    out dx,al     	

    ; switch to PM
    cli
    mov eax, cr0                          ; set bit 0 in cr0 --> enter PM
    or eax, 1
    mov cr0, eax
    jmp DWORD CODE_DESC:ProtectedMode     ; far jump to fix CS. Remember that the code selector is 0x8!

[Bits 32]
ProtectedMode:
    mov ax, DATA_DESC	                  ; set data segments to data selector (0x10)
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov esp, 0x9FC00     ; ATTENTION: don't use it for VBE struct (0x9000) !!!

CopyImage:
    mov eax, DWORD [ImageSize]
    movzx ebx, WORD  [BytesPerSec]
    mul ebx
    mov ebx, 4
    div ebx
    cld
    mov esi, IMAGE_RMODE_BASE
    mov edi, IMAGE_PMODE_BASE
    mov ecx, eax
    rep movsd                             ; copy image to its protected mode address

;*******************************************************
;   Execute Kernel
;*******************************************************
EXECUTE:
;    jmp DWORD CODE_DESC:IMAGE_PMODE_BASE
    jmp [IMAGE_PMODE_BASE]

;*******************************************************
;   calls, e.g. print_string
;*******************************************************
BITS 16
print_string:
   lodsb          ; grab a byte from SI
   or al, al      ; logical or AL by itself
   jz .done       ; if the result is zero, get out
   mov ah, 0x0E
   int 0x10       ; otherwise, print out the character!
   jmp print_string
.done:
   ret
