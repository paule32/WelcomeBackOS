#include "os.h"
#include "descriptor_tables.h"

// Internal function prototypes.
void write_tss(int,USHORT,ULONG);

tss_entry_t tss;


// Initialise our task state segment structure.
void write_tss(int num, USHORT ss0, ULONG esp0)
{
    // Firstly, let's compute the base and limit of our entry into the GDT.
    ULONG base = (ULONG) &tss;
    ULONG limit = sizeof(tss); //http://forum.osdev.org/viewtopic.php?f=1&t=19819&p=155587&hilit=tss_entry#p155587

    // Now, add our TSS descriptor's address to the GDT.
    gdt_set_gate(num, base, limit, 0xE9, 0x00);

    // Ensure the descriptor is initially zero.
    k_memset(&tss, 0, sizeof(tss));

    tss.ss0  = ss0;  // Set the kernel stack segment.
    tss.esp0 = esp0; // Set the kernel stack pointer.

    tss.cs = 0x18;
    tss.ss = 0x10;
    tss.ds = 0x10;
    //tss.es = 0x10; //
    //tss.fs = 0x10; //
    tss.gs = 0x10;
}

void set_kernel_stack(ULONG stack)
{
    tss.esp0 = stack;
}
