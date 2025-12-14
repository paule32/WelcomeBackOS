// gdt.c – Kernel-GDT mit Usermode + TSS
# include "stdint.h"
# include "gdt.h"

// GDT-Eintrag
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

// GDT-Pointer für lgdt
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// TSS-Struktur (vereinfachte Variante)
struct tss_entry_struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

static struct gdt_entry gdt[6];
static struct gdt_ptr   gp;
static struct tss_entry_struct tss_entry;

extern void gdt_flush(uint32_t); // in ASM
extern void tss_flush(void);     // in ASM

static void gdt_set_gate(int num, uint32_t base, uint32_t limit,
                         uint8_t access, uint8_t gran)
{
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

static void write_tss(int num, uint16_t ss0, uint32_t esp0)
{
    uint32_t base  = (uint32_t)&tss_entry;
    uint32_t limit = sizeof(struct tss_entry_struct) - 1;

    gdt_set_gate(num, base, limit, 0x89, 0x40);  // G=0, D/B=0 für TSS
    // 0x89: Present, DPL=0, Typ=1001b (32-bit TSS)

    // TSS leeren
    for (uint32_t
        *p = (uint32_t*)(&tss_entry);
         p < (uint32_t*)(&tss_entry + 1);
         ++p) {
        *p = 0;
    }

    tss_entry.ss0 = 0x10; // ss0;
    tss_entry.esp0 = esp0;
    tss_entry.cs = 0x1b;  // usermode cs (0x1B) niedrige 16 Bit
    tss_entry.ss = 0x23;  // usermode ss (0x23)
    tss_entry.ds = 0x23;
    tss_entry.es = 0x23;
    tss_entry.fs = 0x23;
    tss_entry.gs = 0x23;
    
    tss_entry.iomap_base = sizeof(tss_entry);
}

void gdt_init(void)
{
    gp.limit = sizeof(gdt) - 1;
    gp.base  = (uint32_t)&gdt;

    // 0: Null-Descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    // 1: Kernel Code (Ring0)
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF);
    // 0x9A: Present, DPL=0, Code-Segment, Executable, Readable
    // 0xCF: 4KiB Granularität, 32-bit

    // 2: Kernel Data (Ring0)
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF);
    // 0x92: Present, DPL=0, Data-Segment, Read/Write

    // 3: User Code (Ring3)
    gdt_set_gate(3, 0, 0xFFFFF, 0xFA, 0xCF);
    // 0xFA: DPL=3, Code

    // 4: User Data (Ring3)
    gdt_set_gate(4, 0, 0xFFFFF, 0xF2, 0xCF);
    // 0xF2: DPL=3, Data

    // 5: TSS
    write_tss(5, 0x10, 0); // ss0 = Kernel Data (0x10), esp0 setzen wir später

    gdt_flush((uint32_t)&gp);
    tss_flush();
}

void tss_set_kernel_stack(uint32_t stack_top)
{
    tss_entry.esp0 = stack_top;
}
