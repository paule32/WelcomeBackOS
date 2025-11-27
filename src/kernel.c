// ---------------------------------------------------------------------------
// \file kernel.c – A simple freestanding C-Kernel
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// no standard library, no syscalls, nothing.
// loader jumps directly to kmain() (Linkerscript ENTRY).
// ---------------------------------------------------------------------------
#include "keyboard.h"

#define INT_MAX  2147483647
#define INT_MIN -2147483648

#define NUMBER_GDT_GATES 5

unsigned char ShiftKeyDown = 0; // Variable for Shift Key Down
unsigned char KeyPressed   = 0; // Variable for Key Pressed
unsigned char scan         = 0; // Scan code from Keyboard

extern void k_display(void) __asm__("_k_display");

extern unsigned char k_getch(void);

extern void k_itoa(int value, char* valuestring);
extern void k_i2hex(unsigned int val, unsigned char* dest, int len);

extern unsigned inportb (unsigned port);
extern void     outportb(unsigned port, unsigned val);

/* Message string corresponding to the exception number 0-31: exception_messages[interrupt_number] */
unsigned char* exception_messages[] =
{
    (unsigned char*)"Division By Zero",        (unsigned char*)"Debug",
    (unsigned char*)"Non Maskable Interrupt",  (unsigned char*)"Breakpoint",
    (unsigned char*)"Into Detected Overflow",  (unsigned char*)"Out of Bounds",
    (unsigned char*)"Invalid Opcode",          (unsigned char*)"No Coprocessor",
    (unsigned char*)"Double Fault",            (unsigned char*)"Coprocessor Segment Overrun",
    (unsigned char*)"Bad TSS",                 (unsigned char*)"Segment Not Present",
    (unsigned char*)"Stack Fault",             (unsigned char*)"General Protection Fault",
    (unsigned char*)"Page Fault",              (unsigned char*)"Unknown Interrupt",
    (unsigned char*)"Coprocessor Fault",       (unsigned char*)"Alignment Check",
    (unsigned char*)"Machine Check",           (unsigned char*)"Reserved",
    (unsigned char*)"Reserved",                (unsigned char*)"Reserved",
    (unsigned char*)"Reserved",                (unsigned char*)"Reserved",
    (unsigned char*)"Reserved",                (unsigned char*)"Reserved",
    (unsigned char*)"Reserved",                (unsigned char*)"Reserved",
    (unsigned char*)"Reserved",                (unsigned char*)"Reserved",
    (unsigned char*)"Reserved",                (unsigned char*)"Reserved"
};

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
};

/* Array of function pointers handling custom IRQ handlers for a given IRQ */
void* irq_routines[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* Implement a custom IRQ handler for the given IRQ */
void irq_install_handler  (int irq, void (*handler)(struct regs* r)) {irq_routines[irq] = handler;}
void irq_uninstall_handler(int irq) {irq_routines[irq] = 0;} /* Clear the custom IRQ handler */

// IDT entry
struct idt_entry
{
    unsigned short base_lo;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_hi;
}   __attribute__((packed)); //prevent compiler optimization

struct idt_ptr
{
    unsigned short limit;
    unsigned int base;
}   __attribute__((packed)); //prevent compiler optimization

// Declare an IDT of 256 entries and a pointer to the IDT
struct idt_entry idt[256];
struct idt_ptr   idt_register;

/* Defines a GDT entry */
struct gdt_entry
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));

struct gdt_ptr
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

/* Our GDT, with 3 entries, and finally our special GDT pointer */
struct gdt_entry gdt[NUMBER_GDT_GATES];
struct gdt_ptr   gdt_register;

static void idt_load(){ asm volatile("lidt %0" : "=m" (idt_register)); }
static void gdt_load(){ asm volatile("lgdt %0" : "=m" (gdt_register)); }

void gdt_install();
void idt_install();
void irq_install();
void isr_install();

extern void timer_install  ();
extern void timer_uninstall();
extern void timer_handler(struct regs* r);

/* Own ISR pointing to individual IRQ handler instead of the regular 'fault_handler' function */
extern void irq0 (); extern void irq1 (); extern void irq2 (); extern void irq3 ();
extern void irq4 (); extern void irq5 (); extern void irq6 (); extern void irq7 ();
extern void irq8 (); extern void irq9 (); extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();

/* These are function prototypes for all of the exception handlers:
 * The first 32 entries in the IDT are reserved, and are designed to service exceptions! */
extern void isr0 (); extern void isr1 (); extern void isr2 (); extern void isr3 ();
extern void isr4 (); extern void isr5 (); extern void isr6 (); extern void isr7 ();
extern void isr8 (); extern void isr9 (); extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();

extern void* k_memcpy(void*, const void*, unsigned int);
extern void* k_memset(void*, char       , unsigned int);

extern unsigned short* k_memsetw(unsigned short, unsigned short, unsigned int);

unsigned int k_printf(char* message, unsigned int line)
{
    char* vidmem = (char*) 0xb8000;
    unsigned int i = line * 80 * 2;

    while(*message != 0) {
        if(*message==0x2F) {
            *message++;
            if(*message==0x6e)
            {
                line++;
                i=(line*80*2);
                *message++;
                if(*message == 0) {
                    return (1);
                }
            }
        }
        vidmem[i] = *message;
        *message++;
        ++i;
        vidmem[i] = 0x7;
        ++i;
    }
    return 1;
}

void set_cursor(int row, int col)
{
    unsigned short    position = (row * 80) + col;
    // cursor LOW port to vga INDEX register
    outportb(0x3D4, 0x0F);
    outportb(0x3D5, (unsigned char)(position & 0xFF));
    
    // cursor HIGH port to vga INDEX register
    outportb(0x3D4, 0x0E);
    outportb(0x3D5, (unsigned char)((position>>8)&0xFF));
}

// ---------------------------------------------------------------------------
// \brief kernel PM-DOS screen clear function with default 80x25 dimension.
// ---------------------------------------------------------------------------
void k_clear_screen()
{
    char* vidmem = (char*) 0xb8000;
    unsigned int i=0;
    while(i < (80*2*25)) {
        vidmem[i  ] = ' ';
        vidmem[i+1] = 0x07;
        i += 2;
    }
}

void kmain(void) {
    unsigned char KeyGot = 0;
    
    char bufferKEY[10];
    char bufferASCII[10];
    char bufferASCII_hex[10];

    k_clear_screen();

    gdt_install();
    idt_install();
    isr_install();
    irq_install();
    
    timer_install();
    
    int i;
    for(i = 0; i < 300000000; ++i)
    {
        int j;
        for(j = 0; j < 300000000; ++j);
        
        k_printf("Welcome to WelcomeBack OS."   , 0);
        k_printf("The C kernel has been loaded.", 1);
        
        set_cursor(4, 0);
        KeyGot = k_getch();   // port 0x60 -> scancode + shift key -> ASCII
       
        bufferKEY[0] = KeyGot;
        
        k_itoa(KeyGot , bufferASCII);
        k_i2hex(KeyGot, (unsigned char*)bufferASCII_hex, 2);

        k_printf("             ", 4);
        k_printf("             ", 5);
        k_printf("             ", 6);
        
        k_printf(bufferKEY,       4); // the ASCII character
        k_printf(bufferASCII,     5); // ASCII decimal
        k_printf(bufferASCII_hex, 6); // ASCII hexadecimal
    }
}

/* Wait until buffer is empty */
void keyboard_init()
{
    while( inportb(0x64)&1 )
        inportb(0x60);
}

unsigned char FetchAndAnalyzeScancode()
{
    if( inportb(0x64)&1 )
        scan = inportb(0x60);   // 0x60: get scan code from the keyboard

    // ACK: toggle bit 7 at port 0x61
    unsigned char port_value = inportb(0x61);
    outportb(0x61,port_value |  0x80); // 0->1
    outportb(0x61,port_value &~ 0x80); // 1->0

    if( scan & 0x80 ) // Key released? Check bit 7 (10000000b = 0x80) of scan code for this
    {
        KeyPressed = 0;
        scan &= 0x7F; // Key was released, compare only low seven bits: 01111111b = 0x7F
        if( scan == KRLEFT_SHIFT || scan == KRRIGHT_SHIFT ) // A key was released, shift key up?
        {
            ShiftKeyDown = 0;    // yes, it is up --> NonShift
        }
    }
    else // Key was pressed
    {
        KeyPressed = 1;
        if( scan == KRLEFT_SHIFT || scan == KRRIGHT_SHIFT )
        {
            ShiftKeyDown = 1; // It is down, use asciiShift characters
        }
    }
    return scan;
}

unsigned char k_getch(void) // Scancode --> ASCII
{
    unsigned char retchar;               // The character that returns the scan code to ASCII code
    scan = FetchAndAnalyzeScancode();    // Grab scancode, and get the position of the shift key

    if( ShiftKeyDown )
        retchar = asciiShift[scan];      // (Upper) Shift Codes
    else
        retchar = asciiNonShift[scan];   // (Lower) Non-Shift Codes

    if( ( !(scan == KRLEFT_SHIFT || scan == KRRIGHT_SHIFT) ) && ( KeyPressed == 1 ) ) //filter Shift Key and Key Release
    return retchar; // ASCII version
    return 0;
}

// Put an entry into the IDT
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
{
    idt[num].base_lo = (base        & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     =   sel;
    idt[num].always0 =     0;
    idt[num].flags   = flags;
}

/* Remap: IRQ0 to IRQ15 have to be remapped to IDT entries 32 to 47 */
void irq_remap()
{
    // starts the initialization sequence
    outportb(0x20, 0x11); outportb(0xA0, 0x11);

    // define the PIC vectors
    outportb(0x21, 0x20); // Set offset of Master PIC to 0x20 (32): Entry 32-39
    outportb(0xA1, 0x28); // Set offset of Slave  PIC to 0x28 (40): Entry 40-47

    // continue initialization sequence
    outportb(0x21, 0x04); outportb(0xA1, 0x02);
    outportb(0x21, 0x01); outportb(0xA1, 0x01);
    outportb(0x21, 0x00); outportb(0xA1, 0x00);
}

void idt_install()
{
    // Sets the special IDT pointer up
    idt_register.limit = (sizeof (struct idt_entry) * 256)-1;
    idt_register.base  = (unsigned int) &idt;

    k_memset(&idt, 0, sizeof(struct idt_entry) * 256); // Clear out the entire IDT

    // Add any new ISRs to the IDT here using idt_set_gate
    // ...

    idt_load(); // The IDT register (IDTR) has to point to the IDT
}

/* Setup a descriptor in the Global Descriptor Table */
void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
    /* Setup the descriptor base address */
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    /* Setup the descriptor limits */
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    /* Finally, set up the granularity and access flags */
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

/* Should be called by main. This will setup the special GDT pointer,
 * set up the first 3 entries in our GDT, and then finally call gdt_load() */
void gdt_install()
{
    /* Setup the GDT pointer and limit */
    gdt_register.limit = (sizeof(struct gdt_entry) * NUMBER_GDT_GATES)-1;
    gdt_register.base  = (unsigned int) &gdt;

    /* GDT GATES -  desriptors with pointers to the linear memory address */
    gdt_set_gate(0, 0, 0, 0, 0);                // NULL descriptor
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // CODE, privilege level 0 for kernel code
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // DATA, privilege level 0 for kernel code
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // CODE, privilege level 3 for user code
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // DATA, privilege level 3 for user code

    gdt_load(); // The GDT register (GDTR) of the CPU points to the GDT
}

/* After remap of the interrupt controllers the appropriate ISRs are connected to the correct entries in the IDT. */
void irq_install()
{
    irq_remap();
    idt_set_gate(32, (unsigned) irq0,  0x08, 0x8E);   idt_set_gate(33, (unsigned) irq1,  0x08, 0x8E);
    idt_set_gate(34, (unsigned) irq2,  0x08, 0x8E);   idt_set_gate(35, (unsigned) irq3,  0x08, 0x8E);
    idt_set_gate(36, (unsigned) irq4,  0x08, 0x8E);   idt_set_gate(37, (unsigned) irq5,  0x08, 0x8E);
    idt_set_gate(38, (unsigned) irq6,  0x08, 0x8E);   idt_set_gate(39, (unsigned) irq7,  0x08, 0x8E);
    idt_set_gate(40, (unsigned) irq8,  0x08, 0x8E);   idt_set_gate(41, (unsigned) irq9,  0x08, 0x8E);
    idt_set_gate(42, (unsigned) irq10, 0x08, 0x8E);   idt_set_gate(43, (unsigned) irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned) irq12, 0x08, 0x8E);   idt_set_gate(45, (unsigned) irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned) irq14, 0x08, 0x8E);   idt_set_gate(47, (unsigned) irq15, 0x08, 0x8E);
}

/*  EOI command to the controllers. If you don't send them, any more IRQs cannot be raised */
void irq_handler(struct regs* r)
{
    /* This is a blank function pointer */
    void (*handler)(struct regs* r);

    /* Find out if we have a custom handler to run for this IRQ, and then finally, run it */
    handler = irq_routines[r->int_no - 32];
    if (handler) { handler(r); }

    /* If the IDT entry that was invoked was greater than 40 (IRQ8 - 15),
    *  then we need to send an EOI to the slave controller */
    if (r->int_no >= 40) { outportb(0xA0, 0x20); }

    /* In either case, we need to send an EOI to the master interrupt controller too */
    outportb(0x20, 0x20);
}

/* Set the first 32 entries in the IDT to the first 32 ISRs. Access flag is set to 0x8E: present, ring 0,
*  lower 5 bits set to the required '14' (hexadecimal '0x0E'). */
void isr_install()
{
    idt_set_gate( 0, (unsigned) isr0, 0x08, 0x8E);    idt_set_gate( 1, (unsigned) isr1, 0x08, 0x8E);
    idt_set_gate( 2, (unsigned) isr2, 0x08, 0x8E);    idt_set_gate( 3, (unsigned) isr3, 0x08, 0x8E);
    idt_set_gate( 4, (unsigned) isr4, 0x08, 0x8E);    idt_set_gate( 5, (unsigned) isr5, 0x08, 0x8E);
    idt_set_gate( 6, (unsigned) isr6, 0x08, 0x8E);    idt_set_gate( 7, (unsigned) isr7, 0x08, 0x8E);
    idt_set_gate( 8, (unsigned) isr8, 0x08, 0x8E);    idt_set_gate( 9, (unsigned) isr9, 0x08, 0x8E);
    idt_set_gate(10, (unsigned)isr10, 0x08, 0x8E);    idt_set_gate(11, (unsigned)isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned)isr12, 0x08, 0x8E);    idt_set_gate(13, (unsigned)isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned)isr14, 0x08, 0x8E);    idt_set_gate(15, (unsigned)isr15, 0x08, 0x8E);
    idt_set_gate(16, (unsigned)isr16, 0x08, 0x8E);    idt_set_gate(17, (unsigned)isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned)isr18, 0x08, 0x8E);    idt_set_gate(19, (unsigned)isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned)isr20, 0x08, 0x8E);    idt_set_gate(21, (unsigned)isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned)isr22, 0x08, 0x8E);    idt_set_gate(23, (unsigned)isr23, 0x08, 0x8E);
    idt_set_gate(24, (unsigned)isr24, 0x08, 0x8E);    idt_set_gate(25, (unsigned)isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned)isr26, 0x08, 0x8E);    idt_set_gate(27, (unsigned)isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned)isr28, 0x08, 0x8E);    idt_set_gate(29, (unsigned)isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned)isr30, 0x08, 0x8E);    idt_set_gate(31, (unsigned)isr31, 0x08, 0x8E);
}

/* The exception handling ISR points to this function. This tells what exception has happened!
*  ISRs disable interrupts while they are being serviced */
void fault_handler(struct regs* r)
{
    if (r->int_no < 32)
    {
        k_printf((char*)exception_messages[r->int_no],    7);
        k_printf("   Exception. System Halted!\n", 8);
        for (;;);
    }
}
