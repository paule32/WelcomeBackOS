// task.c
// Very simple round-robin task scheduler for kernel tasks
// (c) 2025 by Jens Kallup - paule32

# include "task.h"
# include "kheap.h"   // für kmalloc
# include "stdint.h"

#define KERNEL_STACK_SIZE   4096

static task_t* current_task = NULL;
static task_t* task_list    = NULL;
static int     task_count   = 0;
static int     tasking_started = 0;

extern void* kmemset   (void*,int        ,uint32_t);

int tasking_is_enabled(void)
{
    return tasking_started;
}

// Hilfsfunktion: Task an Liste anhängen
static void task_append(task_t* t)
{
    if (!task_list) {
        task_list = t;
        t->next   = t; // zirkuläre Liste mit einem Element
    } else {
        // in zirkuläre Liste einhängen (vor task_list)
        task_t* tmp = task_list;
        while (tmp->next != task_list)
            tmp = tmp->next;

        tmp->next = t;
        t->next   = task_list;
    }
    task_count++;
}

// Wird aus kmain() aufgerufen, bevor interrupts aktiviert werden.
// Legt einen "Haupttask" an, der den aktuellen Kernel-Kontext repräsentiert.
// Der Registerzustand dieses Haupttasks wird beim ersten schedule()-Aufruf
// aus dem IRQ heraus gesetzt.
void tasking_init(void)
{
    // Haupttask anlegen
    task_t* main_task = (task_t*)kmalloc(sizeof(task_t));
    kmemset(&main_task->regs, 0, sizeof(regs_t));
    main_task->next = NULL;

    task_append(main_task);
    current_task = main_task;
    tasking_started = 0;   // wird im ersten schedule() == "bewaffnet"
}

// Legt einen neuen Kernel-Task an, der in 'entry' startet
void task_create(void (*entry)(void))
{
    task_t* t = (task_t*)kmalloc(sizeof(task_t));
    if (!t) return;

    kmemset(&t->regs, 0, sizeof(regs_t));

    // Einfacher Kernel-Stack
    uint8_t* stack = (uint8_t*)kmalloc(KERNEL_STACK_SIZE);
    if (!stack) return;

    uint32_t stack_top = (uint32_t)(stack + KERNEL_STACK_SIZE);

    // regs_t Layout (siehe isr.h):
    // wir müssen mindestens diese Felder korrekt setzen:
    //
    // regs.eip, regs.cs, regs.eflags, regs.esp, regs.ss
    //
    // Der Rest ist erstmal 0 (wird bei Bedarf genutzt).

    t->regs.eip     = (uint32_t)entry;
    t->regs.cs      = 0x08;      // Kernel Code Segment
    t->regs.eflags  = 0x202;     // IF=1, Bit 9 gesetzt
    t->regs.esp     = stack_top;
    t->regs.ss      = 0x10;      // Kernel Data/Stack Segment

    // Segment-Register
    t->regs.ds = 0x10;
    t->regs.es = 0x10;
    t->regs.fs = 0x10;
    t->regs.gs = 0x10;

    t->next = NULL;
    task_append(t);
}

// Wird von irq_handler() bei IRQ0 (Timer) aufgerufen.
// r zeigt auf das aktuell gesicherte Register-Frame dieses Interrupts.
void schedule(regs_t* r)
{
    if (!current_task || task_count == 0)
        return;

    if (!tasking_started) {
        // Erster Aufruf: aktuellen Kontext als main_task speichern
        current_task->regs = *r;
        tasking_started = 1;
        return;
    }

    // Aktuellen Kontext speichern
    current_task->regs = *r;

    // Nächsten Task wählen (Round-Robin)
    current_task = current_task->next;
    if (!current_task) {
        current_task = task_list;
    }

    // Kontext des neuen Tasks in r schreiben
    *r = current_task->regs;
}
