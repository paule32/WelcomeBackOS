#ifndef TASK_H
#define TASK_H

#include "os.h"
#include "paging.h"
#include "descriptor_tables.h"

#define KERNEL_STACK_SIZE 2048       // Use a 2kb kernel stack.

extern ULONG initial_esp;

typedef struct task
{
    int id;                           // Process ID.
    ULONG esp, ebp;                   // Stack and base pointers.
    ULONG eip;                        // Instruction pointer.
    ULONG ss;
    page_directory_t* page_directory; // Page directory.
    ULONG kernel_stack;               // Kernel stack location.
    struct task* next;                // The next task in a linked list.
} task_t;

void tasking_install();
ULONG task_switch(ULONG esp);
int fork();
int getpid();
task_t* create_task (void* entry, unsigned char privilege);
void switch_context();

void task_log(task_t* t);
void TSS_log(tss_entry_t* tss);
void log_task_list();

#endif
