// idt.c
#include "stdint.h"
#include "idt.h"

// 256 mögliche IDT-Einträge (0..255)
static idt_entry_t idt_entries[256];
static idt_ptr_t   idt_ptr;

extern void idt_flush(uint32_t);  // in ASM

// base = Adresse des Handlers
// sel  = Code-Segment-Selector (z.B. 0x08)
// flags = 0x8E für Interrupt-Gate, Kernel; 0xEE für DPL=3 (Syscall)
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;

    idt_entries[num].sel       = sel;
    idt_entries[num].always0   = 0;
    idt_entries[num].flags     = flags;
}

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void isr128();  // int 0x80 (syscall)

void idt_init(void)
{
    for(;;);
    /*
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // alles auf 0
    for (int i = 0; i < 256; ++i) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Kernel-Code-Segment: 0x08 (aus deiner GDT)
    uint16_t kcode_sel = 0x08;

    // Exceptions 0..31 (Interrupt-Gates, DPL=0)
    idt_set_gate( 0, (uint32_t)isr0,  kcode_sel, 0x8E);
    idt_set_gate( 1, (uint32_t)isr1,  kcode_sel, 0x8E);
    idt_set_gate( 2, (uint32_t)isr2,  kcode_sel, 0x8E);
    idt_set_gate( 3, (uint32_t)isr3,  kcode_sel, 0x8E);
    idt_set_gate( 4, (uint32_t)isr4,  kcode_sel, 0x8E);
    idt_set_gate( 5, (uint32_t)isr5,  kcode_sel, 0x8E);
    idt_set_gate( 6, (uint32_t)isr6,  kcode_sel, 0x8E);
    idt_set_gate( 7, (uint32_t)isr7,  kcode_sel, 0x8E);
    idt_set_gate( 8, (uint32_t)isr8,  kcode_sel, 0x8E);
    idt_set_gate( 9, (uint32_t)isr9,  kcode_sel, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, kcode_sel, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, kcode_sel, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, kcode_sel, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, kcode_sel, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, kcode_sel, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, kcode_sel, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, kcode_sel, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, kcode_sel, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, kcode_sel, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, kcode_sel, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, kcode_sel, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, kcode_sel, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, kcode_sel, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, kcode_sel, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, kcode_sel, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, kcode_sel, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, kcode_sel, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, kcode_sel, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, kcode_sel, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, kcode_sel, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, kcode_sel, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, kcode_sel, 0x8E);

    // Syscall-Interrupt 0x80:
    // Flags: 0xEE = 1110 1110b
    //  - P=1 (present)
    //  - DPL=3 (Usermode darf aufrufen)
    //  - Type=1110b = 32-Bit Interrupt-Gate
    idt_set_gate(0x80, (uint32_t)isr128, kcode_sel, 0xEE);

    // IDT der CPU mitteilen
    idt_flush((uint32_t)&idt_ptr);*/
}
