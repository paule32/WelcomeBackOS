# include "stdint.h"
# include "idt.h"
# include "isr.h"
# include "task.h"

# define PIC1        0x20
# define PIC2        0xA0

# define PIC1_CMD    PIC1
# define PIC2_CMD    PIC2

# define PIC1_DATA   (PIC1+1)
# define PIC2_DATA   (PIC2+1)

# define ICW1_INIT   0x10
# define ICW1_ICW4   0x01
# define ICW4_8086   0x01

extern void timer_tick();

// einfache Portfunktionen
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void irq_remap(void)
{
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    // Initialisierung beider PICs
    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);

    // Master-PIC Offset = 0x20
    outb(PIC1_DATA, 0x20);

    // Slave-PIC Offset = 0x28
    outb(PIC2_DATA, 0x28);

    // PIC-Verschaltung festlegen
    outb(PIC1_DATA, 4);  // sagt Master: da hängt ein Slave an IRQ2
    outb(PIC2_DATA, 2);  // sagt Slave: er hängt an IRQ2 des Masters

    // ICW4 setzen
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // alte Masken wiederherstellen
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

void irq_handler(regs_t* r)
{
    if (r->int_no == 32) {
        // Timer IRQ
        timer_tick();  // optional: Tick-Zähler weiterlaufen lassen

        if (tasking_is_enabled()) {
            schedule(r);
        }
    }

    // EOI schicken
    if (r->int_no >= 40)
    outb(PIC2_CMD, 0x20);
    outb(PIC1_CMD, 0x20);
}

void irq_init(void)
{
    irq_remap();

    // IRQ0 - Timer
    extern void irq0();
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);

    // IRQ1 - Keyboard
    extern void irq1();
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
}
