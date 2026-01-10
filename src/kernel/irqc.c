// ----------------------------------------------------------------------------
// \file  irqc.c
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"
# include "idt.h"
# include "isr.h"
# include "task.h"

# define KERNEL_CS   0x08

# define PIC1        0x20
# define PIC2        0xA0

# define PIC_EOI     0x20

# define PIC1_CMD    PIC1
# define PIC2_CMD    PIC2

# define PIC1_DATA   (PIC1+1)
# define PIC2_DATA   (PIC2+1)

# define ICW1_INIT   0x10
# define ICW1_ICW4   0x01
# define ICW4_8086   0x01

extern void timer_tick();

extern void irq12_stub(void);          // asm stub
//extern void idt_set_gate(int n, uint32_t base, uint16_t sel, uint8_t flags);

// einfache Portfunktionen
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void io_wait(void)
{
    // klassisch: kurzer Delay über Port 0x80
    __asm__ volatile ("outb %%al, $0x80" :: "a"(0));
}

static inline uint32_t irq_save(void)
{
    uint32_t flags;
    __asm__ volatile (
        "pushfl\n\t"
        "popl %0\n\t"
        "cli\n\t"
        : "=r"(flags) :: "memory"
    );
    return flags;
}
static inline void irq_restore(uint32_t flags)
{
    __asm__ volatile (
        "pushl %0\n\t"
        "popfl\n\t"
        :: "r"(flags) : "memory", "cc"
    );
}

static void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
    outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}
static void pic_irq_unmask(uint8_t irq)
{
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t  bit  = (irq < 8) ? irq : (irq - 8);

    uint8_t mask = inb(port);
    mask &= (uint8_t)~(1u << bit);
    outb(port, mask);
}

void irq_remap(uint8_t off1, uint8_t off2)
{
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    // Initialisierung beider PICs
    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4); io_wait();
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4); io_wait();

    
    outb(PIC1_DATA, off1); io_wait();  // Master-PIC Offset = 0x20
    outb(PIC2_DATA, off2); io_wait();  // Slave-PIC Offset = 0x28

    // PIC-Verschaltung festlegen
    outb(PIC1_DATA, 0x04); io_wait(); // sagt Master: da hängt ein Slave an IRQ2
    outb(PIC2_DATA, 0x02); io_wait(); // sagt Slave: er hängt an IRQ2 des Masters

    // ICW4 setzen
    outb(PIC1_DATA, ICW4_8086); io_wait();  // ICW4: 8086 mode
    outb(PIC2_DATA, ICW4_8086); io_wait();

    // alte Masken wiederherstellen
    outb(PIC1_DATA, a1); // restore masks
    outb(PIC2_DATA, a2);
}

// ----------------------------------------------------------------------------
// timer handler ...
// ----------------------------------------------------------------------------
void irq0_handler_c(regs_t* r)
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

// ----------------------------------------------------------------------------
// keyboard handler ...
// ----------------------------------------------------------------------------
typedef struct {
    uint8_t shift, ctrl, alt, caps;
} kbd_mod_t;

typedef struct {
    uint8_t scancode;   // "code" ohne 0x80-Breakbit (0..127)
    uint8_t released;   // 0 = press, 1 = release
    uint8_t e0;         // 1 wenn E0-prefix (Cursor, Insert, etc.)
    uint8_t ascii;      // 0 wenn nicht übersetzt/keins
    kbd_mod_t mod;      // Modifier-Snapshot zum Zeitpunkt des Events
}   key_event_t;

# define KEYQ_SIZE  128u                 // MUSS power-of-two sein
# define KEYQ_MASK  (KEYQ_SIZE - 1u)

typedef struct {
    volatile uint32_t head;             // write index (Producer)
    volatile uint32_t tail;             // read index (Consumer)
    key_event_t buf[KEYQ_SIZE];
    volatile uint32_t dropped;          // Statistik: verworfene Events
}   key_queue_t;

static key_queue_t g_keyq = {0};
static inline bool keyq_push_isr(const key_event_t *ev)
{
    uint32_t head = g_keyq.head;
    uint32_t next = (head + 1u) & KEYQ_MASK;

    if (next == g_keyq.tail) {
        g_keyq.dropped++;
        return false; // voll -> verworfen
    }

    g_keyq.buf[head] = *ev;
    g_keyq.head = next;
    return true;
}
bool keyq_pop(key_event_t *out)
{
    uint32_t flags = irq_save();

    uint32_t tail = g_keyq.tail;
    if (tail == g_keyq.head) {
        irq_restore(flags);
        return false; // leer
    }

    *out = g_keyq.buf[tail];
    g_keyq.tail = (tail + 1u) & KEYQ_MASK;

    irq_restore(flags);
    return true;
}

bool keyq_empty(void)
{
    uint32_t flags = irq_save();
    bool empty = (g_keyq.tail == g_keyq.head);
    irq_restore(flags);
    return empty;
}
static const uint8_t keymap[128] = {
    /* 0x00 */ 0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    /* 0x0F */ '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    /* 0x1E */ 'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
    /* 0x2C */ 'z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',
    // Rest 0
};

static const uint8_t keymap_shift[128] = {
    /* 0x00 */ 0,  27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    /* 0x0F */ '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,
    /* 0x1E */ 'A','S','D','F','G','H','J','K','L',':','"','~',0,'|',
    /* 0x2C */ 'Z','X','C','V','B','N','M','<','>','?',0,'*',0,' ',
};

static inline uint8_t kbd_translate_ascii(uint8_t sc, kbd_mod_t mod)
{
    // press-only in der Regel; releases liefern ascii=0
    bool shifted = (mod.shift != 0);

    // Caps wirkt meist nur auf Buchstaben: hier simpel gehalten
    uint8_t ch = shifted ? keymap_shift[sc] : keymap[sc];

    if (mod.caps && ch >= 'a' && ch <= 'z') ch = (uint8_t)(ch - 'a' + 'A');
    if (mod.caps && ch >= 'A' && ch <= 'Z') ch = (uint8_t)(ch - 'A' + 'a');

    return ch;
}
// ----------------------------------------------------------------------------
// 8042 Controller
// ----------------------------------------------------------------------------
# define KBD_DATA   0x60
# define KBD_STAT   0x64

static volatile kbd_mod_t kbd_mod = {0};

static inline void kbd_update_modifiers(uint8_t sc)
{
    switch (sc) {
        case 0x2A: case 0x36: kbd_mod.shift = 1; break;
        case 0xAA: case 0xB6: kbd_mod.shift = 0; break;

        case 0x1D: kbd_mod.ctrl = 1; break;
        case 0x9D: kbd_mod.ctrl = 0; break;

        case 0x38: kbd_mod.alt = 1; break;
        case 0xB8: kbd_mod.alt = 0; break;

        case 0x3A: kbd_mod.caps ^= 1; break;
        default: break;
    }
}
// optional: E0-prefix support (Pfeiltasten, etc.)
static volatile uint8_t kbd_e0 = 0;

// Beispiel: hier würdest du in deine Event-Queue pushen
static void kbd_on_key(uint8_t sc, bool released, bool e0, kbd_mod_t mod)
{
    key_event_t ev;
    
    ev.scancode  = sc;
    ev.released  = released ? 1 : 0;
    ev.e0        = e0 ? 1 : 0;
    ev.mod       = mod;
    ev.ascii     = 0;

    // ASCII typischerweise nur bei press und ohne E0
    if (!released && !e0) {
        ev.ascii = kbd_translate_ascii(sc, mod);
    }

    keyq_push_isr(&ev);
}
// ----------------------------------------------------------------------------
// Beispiel: EventLoop liest Queue ...
// ----------------------------------------------------------------------------
void input_poll(void)
{
    key_event_t ev;

    while (keyq_pop(&ev)) {
        // Beispiel: ASCII Zeichen
        if (!ev.released && ev.ascii) {
            // putc(ev.ascii);
        }

        // Beispiel: raw scancode + modifier
        // handle_key_event(ev);
    }
}

void irq1_handler_c(void)
{
    // Status kann man lesen, aber entscheidend ist: Datenport 0x60 auslesen
    uint8_t status = inb(KBD_STAT);
    (void)status;

    uint8_t sc = inb(KBD_DATA);

    // E0/E1 Prefix behandeln (minimal)
    if (sc == 0xE0) { kbd_e0 = 1; pic_send_eoi(1); return; }
    if (sc == 0xE1) { 
        // Pause/Break ist komplex (mehrere Bytes). Minimal:
        // du könntest hier state machine starten. Fürs Erste ignorieren:
        pic_send_eoi(1); return;
    }

    bool released = (sc & 0x80) != 0;
    uint8_t code  = (sc & 0x7F);

    // Modifier anhand des *vollen* Scancodes (inkl. break bit) updaten:
    // (damit 0xAA/0xB6/0x9D/0xB8 funktionieren)
    kbd_update_modifiers(sc);

    // Normale Taste melden (auch releases, je nach deinem Event-System)
    kbd_on_key(code, released, (kbd_e0 != 0), kbd_mod);

    kbd_e0 = 0;

    // EOI an PIC
    pic_send_eoi(1);
}

void irq_init(void)
{
    irq_remap(0x20, 0x28);

    // IRQ0 - Timer
    extern void irq0();
    idt_set_gate(0x20, (uint32_t)irq0, KERNEL_CS, 0x8E);

    // IRQ1 - Keyboard
    extern void irq1();
    idt_set_gate(0x21, (uint32_t)irq1, KERNEL_CS, 0x8E);
    
    pic_irq_unmask(1);
    
    // Interrupts global an
    __asm__ volatile ("sti");
    
    // IRQ12 - Mouse
    //extern void irq12();
    //idt_set_gate(34, (uint32_t)irq12, 0x08, 0x8E);
}
