# include "stdint.h"
# include "syscall.h"
# include "idt.h"
# include "isr.h"

static volatile char* const VGA = (volatile char*)0xB8000;

static void vga_putc(char c)
{
    static int pos = 160; // zweite Zeile (je 2 Byte pro Zeichen)
    if (pos >= 80 * 2 * 10) pos = 160; // primitive "Zeilenbegrenzung"

    VGA[pos]     = c;
    VGA[pos + 1] = 0x07;
    pos += 2;
}

void syscall_init(void)
{
    // Hier wäre später Platz für eine Syscall-Tabelle etc.
    // aktuell nur Platzhalter, eigentliche IDT-Eintragung erfolgt bereits in idt_init()
}

void syscall_dispatch(regs_t* r)
{
    // Konvention: EAX = Syscall ID, EBX, ECX, EDX = Parameter
    uint32_t num = r->eax;

    switch (num) {
        case SYSCALL_PUTCHAR:
            vga_putc((char)r->ebx);   // Beispiel: EBX = Zeichen
            break;

        default:
            // unbekannter Syscall
            vga_putc('?');
            break;
    }
}



#if 0
// some functions declared extern in os.h
// rest of functions must be declared here:
extern void f4();

DEFN_SYSCALL1( puts,                       0, unsigned char*               )
DEFN_SYSCALL1( putch,                      1, unsigned char                )
DEFN_SYSCALL2( settextcolor,               2, unsigned char, unsigned char )
DEFN_SYSCALL0( getpid,                     3                               )
DEFN_SYSCALL0( nop,                        4                               )
DEFN_SYSCALL0( switch_context,             5                               )
DEFN_SYSCALL0( k_checkKQ_and_print_char,   6                               )
DEFN_SYSCALL0( k_checkKQ_and_return_char,  7                               )

#define NUM_SYSCALLS 8

static void* syscalls[NUM_SYSCALLS] =
{
    &puts,
    &putch,
    &settextcolor,
    &getpid,
    &nop,
    &switch_context,
    &k_checkKQ_and_print_char,
    &k_checkKQ_and_return_char
};

void syscall_handler(struct regs* r)
{
    // Firstly, check if the requested syscall number is valid. The syscall number is found in EAX.
    if( r->eax >= NUM_SYSCALLS )
        return;

    void* addr = syscalls[r->eax]; // Get the required syscall location.

    // We don't know how many parameters the function wants, so we just push them all onto the stack in the correct order.
    // The function will use all the parameters it wants, and we can pop them all back off afterwards.
    int ret;
    asm volatile (" \
      push %1; \
      push %2; \
      push %3; \
      push %4; \
      push %5; \
      call *%6; \
      add $20, %%esp; \
    " : "=a" (ret) : "r" (r->edi), "r" (r->esi), "r" (r->edx), "r" (r->ecx), "r" (r->ebx), "r" (addr));
    r->eax = ret;
}
#endif
