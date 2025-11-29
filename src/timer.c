#include "os.h"

//TASKSWITCH
#include "task.h"
extern ULONG read_eip();
extern page_directory_t* current_directory;
extern task_t* current_task;
extern tss_entry_t tss_entry;

ULONG timer_ticks = 0;
ULONG eticks;

void timer_handler(struct regs* r)
{
    ++timer_ticks;
    if(eticks)
        --eticks;
}

void timer_wait (ULONG ticks)
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

void sleepSeconds (ULONG seconds)
{
    timer_wait((ULONG)100*seconds);
}

void sleepMilliSeconds (ULONG ms)
{
    timer_wait((ULONG)(ms/10));
}

void systemTimer_setFrequency( ULONG freq )
{
    ULONG divisor = 1193180 / freq; //divisor must fit into 16 bits

    //TODO: save frequency globally

    // Send the command byte
    outportb(0x43, 0x36);

    // Send divisor
    outportb(0x40, (unsigned char)(  divisor     & 0xFF )); // low  byte
    outportb(0x40, (unsigned char)( (divisor>>8) & 0xFF )); // high byte
}

void timer_install()
{
    /* Installs 'timer_handler' to IRQ0 */
    irq_install_handler(0, timer_handler);
    systemTimer_setFrequency( 100 ); // 100 Hz, meaning a tick every 10 milliseconds
}

void timer_uninstall()
{
    /* Uninstalls IRQ0 */
    irq_uninstall_handler(0);
}

