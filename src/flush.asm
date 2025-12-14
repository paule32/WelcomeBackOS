BITS 32
section .text

global _gdt_flush
_gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]

    mov ax, 0x10       ; Kernel Data Segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far-Jump in Kernel Code Segment (0x08)
    jmp 0x08:.flush_done

.flush_done:
    ret

global _idt_flush
_idt_flush:
    ; cdecl: Rücksprungadresse liegt bei [esp]
    ; erstes Argument bei [esp+4]
    mov eax, [esp+4]   ; EAX = Adresse von idt_ptr (struct idt_ptr)
    lidt [eax]         ; IDT in die CPU laden
    ret

global _tss_flush
_tss_flush:
    mov ax, 0x28      ; TSS-Selector: Index 5, RPL=3 → 5*8 + 3 = 0x2B
    ltr ax
    ret
