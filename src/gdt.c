// gdt.c – Kernel-GDT mit Usermode + TSS
# include "stdint.h"
# include "gdt.h"

// GDT-Eintrag
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;      // Kernel-Stack (oben)
    uint32_t ss0;       // Segment für esp0
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t cs;
    uint16_t ss;
    uint16_t ds;
    uint16_t fs;
    uint16_t gs;
    uint16_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

extern void _gdt_flush(uint32_t);
extern void _tss_flush(void);

#define GDT_ENTRIES 6

gdt_entry_t gdt[GDT_ENTRIES];
gdt_ptr_t   gp;
tss_entry_t tss_entry;

static void gdt_set_gate(
         int num,
    uint32_t base,
    uint32_t limit,
     uint8_t access,
     uint8_t gran) {
    
    gdt[num].limit_low  = (uint16_t)(limit & 0xFFFF);
    gdt[num].base_low   = (uint16_t)(base & 0xFFFF);
    gdt[num].base_middle= (uint8_t)((base >> 16) & 0xFF);
    gdt[num].access     = access;
    gdt[num].granularity= (uint8_t)(((limit >> 16) & 0x0F) | (gran & 0xF0));
    gdt[num].base_high  = (uint8_t)((base >> 24) & 0xFF);
}

void tss_set_kernel_stack(uint32_t stack_top) {
    tss_entry.esp0 = stack_top;
}

static void write_tss(int num, uint16_t ss0, uint32_t esp0)
{
    uint32_t base  = (uint32_t)&tss_entry;
    uint32_t limit = sizeof(tss_entry_t) - 1;

    // TSS-Descriptor: Typ 0x9 (32-bit TSS, verfügbar), Present, DPL=0
    gdt_set_gate(num, base, limit, 0x89, 0x00);

    // TSS komplett nullen
    uint8_t* p = (uint8_t*)&tss_entry;
    for (uint32_t i = 0; i < sizeof(tss_entry_t); ++i)
        p[i] = 0;
    
    // Nur das, was du wirklich brauchst:
    tss_entry.ss0  = ss0;        // Kernel Data Segment (0x10)
    tss_entry.esp0 = esp0;       // Kernel-Stack (oberes Ende)
    tss_entry.iomap_base = sizeof(tss_entry_t);
}

void gdt_init(uint32_t kernel_stack_top)
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
    //extern uint32_t kernel_stack_top; // oder in kmain ermitteln und übergeben
    write_tss(5, 0x10, kernel_stack_top);

    _gdt_flush((uint32_t)&gp);
    _tss_flush();
    
    for(;;);
}