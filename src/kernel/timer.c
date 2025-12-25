# include "stdint.h"

static uint32_t ticks = 0;

void timer_tick()
{
    ticks++;

    // debug: jede Sekunde ein Zeichen ausgeben
    if (ticks % 100 == 0) {
        volatile char* VGA = (volatile char*)0xB8000;
        VGA[160] = 'T';
        VGA[161] = 0x0F;
    }
}
