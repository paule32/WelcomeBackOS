#include "os.h"
#include "descriptor_tables.h"

#define NUMBER_GDT_GATES 6 // 0-5: Null, Kernel Code, Kernel Data, User Code, User Data, TSS

/* Our GDT, and finally our special GDT pointer */
gdt_entry_t gdt[NUMBER_GDT_GATES];
gdt_ptr_t   gdt_register;

/* Setup a descriptor in the Global Descriptor Table */
void gdt_set_gate(int num, ULONG base, ULONG limit, unsigned char access, unsigned char gran)
{
    /* Setup the descriptor base address */
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    /* Setup the descriptor limits */
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    /* Finally, set up the granularity and access flags */
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access      = access;
}

void gdt_install()
{
    /* Setup the GDT pointer and limit */
    gdt_register.limit = (sizeof(struct gdt_entry) * NUMBER_GDT_GATES)-1;
    gdt_register.base  = (ULONG) &gdt;

    /* GDT GATES -  desriptors with pointers to the linear memory address */
    gdt_set_gate(0, 0, 0, 0, 0);                // NULL descriptor
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // CODE, privilege level 0 for kernel code
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // DATA, privilege level 0 for kernel code
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

    write_tss(5, 0x10, 0x0);                    // num, ss0, esp0

    gdt_flush((ULONG)&gdt_register);
    tss_flush();
}
