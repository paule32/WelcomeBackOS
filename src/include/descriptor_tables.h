#ifndef DESCRIPTOR_TABLES_H
#define DESCRIPTOR_TABLES_H

#include "os.h"

void write_tss(int num, USHORT ss0, ULONG esp0);

// asm functions in flush.asm
extern void gdt_flush(ULONG);
extern void tss_flush();
extern void idt_flush(ULONG);

// Initialisation function is publicly accessible.
void init_descriptor_tables();

// Allows the kernel stack in the TSS to be changed.
void set_kernel_stack(ULONG stack);

/* Defines a GDT entry */
struct gdt_entry
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
}__attribute__((packed));

struct gdt_ptr
{
    USHORT limit;
    ULONG   base;
}__attribute__((packed));

typedef struct gdt_entry gdt_entry_t;
typedef struct gdt_ptr   gdt_ptr_t;

// IDT entry
struct idt_entry
{
    USHORT base_lo;
    USHORT sel;
    unsigned char always0;
    unsigned char flags;
    USHORT base_hi;
}__attribute__((packed)); //prevent compiler optimization

struct idt_ptr
{
    USHORT limit;
    ULONG  base;
}__attribute__((packed)); //prevent compiler optimization

typedef struct idt_entry idt_entry_t;
typedef struct idt_ptr   idt_ptr_t;

// Task State Segment (TSS)
struct tss_entry_struct
{
    ULONG prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
    ULONG esp0;       // The stack pointer to load when we change to kernel mode.
    ULONG ss0;        // The stack segment to load when we change to kernel mode.
    ULONG esp1;       // Unused...
    ULONG ss1;
    ULONG esp2;
    ULONG ss2;
    ULONG cr3;
    ULONG eip;
    ULONG eflags;
    ULONG eax;
    ULONG ecx;
    ULONG edx;
    ULONG ebx;
    ULONG esp;
    ULONG ebp;
    ULONG esi;
    ULONG edi;
    ULONG es;         // The value to load into ES when we change to kernel mode.
    ULONG cs;         // The value to load into CS when we change to kernel mode.
    ULONG ss;         // The value to load into SS when we change to kernel mode.
    ULONG ds;         // The value to load into DS when we change to kernel mode.
    ULONG fs;         // The value to load into FS when we change to kernel mode.
    ULONG gs;         // The value to load into GS when we change to kernel mode.
    ULONG ldt;        // Unused...
    USHORT trap;
    USHORT iomap_base;

} __attribute__((packed));

typedef struct tss_entry_struct tss_entry_t;

#endif
