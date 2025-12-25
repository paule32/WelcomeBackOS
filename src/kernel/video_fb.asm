; ---------------------------------------------------------------------------
; \file  putpixel.asm
; \note  (c) 2025 by Jens Kallup - paule32
;        all rights reserved.
; ---------------------------------------------------------------------------
bits 32
section .text

extern _lfb_base
extern _lfb_pitch
extern _lfb_xres
extern _lfb_yres
extern _lfb_bpp
extern _color16

global _init_vbe
_init_vbe:
    ret

; ------------------------------------------------------------
; Funktion: putpixel16
; Eingaben:
;   ECX = x
;   EBX = y
;   [color16] = 16-bit Farbe (RGB565)
;   [lfb_base], [lfb_pitch], [lfb_xres], [lfb_yres]
; ------------------------------------------------------------
global _putpixel16
_putpixel16:
    ; Bounds-Check (einfach)
    cmp ecx, 0
    jl  .ret
    cmp ebx, 0
    jl  .ret
    mov eax, [_lfb_xres]
    cmp ecx, eax
    jge .ret
    mov eax, [_lfb_yres]
    cmp ebx, eax
    jge .ret

    ; offset = y * pitch + x * 2
    mov eax, [_lfb_pitch]   ; eax = pitch
    imul eax, ebx          ; eax = y * pitch

    mov edx, ecx
    shl edx, 1             ; x * 2 (16 bpp)
    add eax, edx           ; eax = offset

    ; Adresse = lfb_base + offset
    mov edx, [_lfb_base]
    add edx, eax           ; edx = Adresse

    ; Farbe schreiben
    mov ax, 0xf880
    mov [edx], ax

    .ret:
    ret
