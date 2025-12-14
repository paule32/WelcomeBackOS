// Simple preemptive kernel tasking using IRQ0
// (c) 2025 by Jens Kallup - paule32

#ifndef TASK_H
#define TASK_H

#include "stdint.h"
#include "isr.h"

typedef struct task {
    regs_t regs;           // saved CPU state
    struct task* next;     // next task in round-robin list
} task_t;

// Initialisiert das Tasking-System (muss im Kernel aufgerufen werden)
extern void tasking_init(void);

// Erzeugt einen neuen Kernel-Task, der bei 'entry' beginnt
extern void task_create(void (*entry)(void));

// Wird vom IRQ-Handler (Timer) aufgerufen, um Task zu wechseln
extern void schedule(regs_t* r);

// Wird vom IRQ-Code benutzt, um festzustellen, ob Tasking aktiv ist
extern int tasking_is_enabled(void);

#endif
