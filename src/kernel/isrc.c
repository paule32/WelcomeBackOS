// isr.c
# include "stdint.h"
# include "isr.h"
# include "idt.h"

/* These are function prototypes for all of the exception handlers:
 * The first 32 entries in the IDT are reserved, and are designed to service exceptions! */
extern void isr0();  extern void isr1();  extern void isr2();  extern void isr3();
extern void isr4();  extern void isr5();  extern void isr6();  extern void isr7();
extern void isr8();  extern void isr9();  extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();

static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

extern void syscall_dispatch(regs_t* r);

// initialisiert nur die ISRs – IDT selbst wird in idt_init() gesetzt
void isr_init(void)
{
    // aktuell nichts extra, IDT-Einträge wurden bereits in idt_init() gesetzt
}

static volatile char* const VGA = (volatile char*)0xB8000;

static void putc_at(int pos, char c, uint8_t color)
{
    VGA[pos * 2]     = c;
    VGA[pos * 2 + 1] = color;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

extern void printformat(const char* fmt, ...);
// sehr simpler Handler
void isr_handler(regs_t* r)
{
    if (r->int_no == 128) {
        // Syscall
        printformat("calller calllers\n");
        syscall_dispatch(r);
        return;
    }

    // Exceptions:
    // Debug-Ausgabe der Interruptnummer
    putc_at(10, 'E', 0x4F);          // 'E' für Exception
    char n = '0' + (char)(r->int_no % 10);
    putc_at(11, n, 0x4F);

    // Hier kannst du später was sinnvolleres tun (Panic-Screen etc.)
    for (;;) {
        asm volatile("hlt");
    }
}
