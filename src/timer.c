/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
};

extern void outportb(unsigned int, unsigned int);
extern void update_cursor();

extern void irq_install();
extern void irq_uninstall_handler(int);

extern void irq_install_handler  (int irq, void (*handler)(struct regs* r));

unsigned long const FREQ  = 100; // 100 "ticks" per second
unsigned long timer_ticks =   0;
unsigned long eticks;

void timer_install();
void timer_uninstall();

void timer_handler(struct regs* r)
{
    ++timer_ticks;
    if (eticks)
        --eticks;
}

void timer_wait (unsigned long ticks)
{
    timer_uninstall();
    eticks = ticks;
    timer_install();

    // busy wait...
    while (eticks)
    {
        update_cursor();
    }
}

void sleepSeconds (unsigned long seconds)
{
    timer_wait(FREQ * seconds);
}

void sleepMilliSeconds (unsigned long ms)
{
    timer_wait(FREQ * ms/1000UL);
}

static void systemTimer_setFrequency( unsigned long freq )
{
   unsigned long divisor = 1193180 / freq; //divisor must fit into 16 bits

   // Send the command byte.
   outportb(0x43, 0x36);

   // Send divisor.
   outportb(0x40, (unsigned char)(  divisor     & 0xFF )); // low  byte
   outportb(0x40, (unsigned char)( (divisor>>8) & 0xFF )); // high byte
}

void timer_install()
{
    /* Installs 'timer_handler' to IRQ0 */
    irq_install_handler(0, timer_handler);
    systemTimer_setFrequency( FREQ ); // FREQ Hz, meaning a tick every 1000/FREQ milliseconds
}

void timer_uninstall()
{
    /* Uninstalls IRQ0 */
    irq_uninstall_handler(0);
}
