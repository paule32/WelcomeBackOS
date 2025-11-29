#include "os.h"
#include "descriptor_tables.h"

// Declare an IDT of 256 entries and a pointer to the IDT
idt_entry_t idt[256];
idt_ptr_t   idt_register;

// Put an entry into the IDT
void idt_set_gate(unsigned char num, ULONG base, USHORT sel, unsigned char flags)
{
    idt[num].base_lo = (base        & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     =   sel;
    idt[num].always0 =     0;
    idt[num].flags   = flags | 0x60;
}

void idt_install()
{
    // Sets the special IDT pointer up
    idt_register.limit = (sizeof (struct idt_entry) * 256)-1;
    idt_register.base  = (ULONG) &idt;

    k_memset(&idt, 0, sizeof(idt_entry_t) * 256); // Clear out the entire IDT

    // Add any new ISRs to the IDT here using idt_set_gate
    // ...

    idt_flush((ULONG)&idt_register); // inclusive idt_load() in assembler code

}

//static void idt_load(){ asm volatile("lidt %0" : "=m" (idt_register)); } // load IDT register (IDTR)
