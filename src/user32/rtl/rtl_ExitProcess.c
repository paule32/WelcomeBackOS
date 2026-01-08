# include "stdint.h"

void ExitProcess(int exitcode)
{
    volatile char* VGA = (volatile char*)0xB8000;
    VGA[10] = 'G'; VGA[11] = 0x0F;
    VGA[12] = 'U'; VGA[13] = 0x0F;
    VGA[14] = 'G'; VGA[15] = 0x0F;
}
