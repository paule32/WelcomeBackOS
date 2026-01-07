# include "stdint.h"

extern uint32_t RtlExitProcess;
void ExitProcess(int exitcode)
{
    typedef void (*cb_int_t)(int);
    ((cb_int_t)(uintptr_t)&RtlExitProcess)(exitcode);
}
