#ifndef SYSCALL_H
#define SYSCALL_H

# include "stdint.h"
# include "isr.h"

//extern void syscall_init(void);           // Initialisiert die Syscall-Schicht
extern void syscall_dispatch(regs_t* r);  // Dispatcher für int 0x80

// ein Beispiel-Syscall: SYSCALL_PUTCHAR (ID 1)
#define SYSCALL_PUTCHAR  1

#endif
