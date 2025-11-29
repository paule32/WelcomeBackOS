#include "os.h"
#include "task.h"
#include "syscall.h"

extern tss_entry_t tss_entry;

/* These are function prototypes for all of the exception handlers:
 * The first 32 entries in the IDT are reserved, and are designed to service exceptions! */
extern void isr0();  extern void isr1();  extern void isr2();  extern void isr3();
extern void isr4();  extern void isr5();  extern void isr6();  extern void isr7();
extern void isr8();  extern void isr9();  extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();

extern void isr127(); //Software interrupt for sys calls

/* Set the first 32 entries in the IDT to the first 32 ISRs. Access flag is set to 0x8E: present, ring 0,
*  lower 5 bits set to the required '14' (hexadecimal '0x0E'). */
void isrs_install()
{
    idt_set_gate( 0, (ULONG) isr0, 0x08, 0x8E);    idt_set_gate( 1, (ULONG) isr1, 0x08, 0x8E);
    idt_set_gate( 2, (ULONG) isr2, 0x08, 0x8E);    idt_set_gate( 3, (ULONG) isr3, 0x08, 0x8E);
    idt_set_gate( 4, (ULONG) isr4, 0x08, 0x8E);    idt_set_gate( 5, (ULONG) isr5, 0x08, 0x8E);
    idt_set_gate( 6, (ULONG) isr6, 0x08, 0x8E);    idt_set_gate( 7, (ULONG) isr7, 0x08, 0x8E);
    idt_set_gate( 8, (ULONG) isr8, 0x08, 0x8E);    idt_set_gate( 9, (ULONG) isr9, 0x08, 0x8E);
    idt_set_gate(10, (ULONG)isr10, 0x08, 0x8E);    idt_set_gate(11, (ULONG)isr11, 0x08, 0x8E);
    idt_set_gate(12, (ULONG)isr12, 0x08, 0x8E);    idt_set_gate(13, (ULONG)isr13, 0x08, 0x8E);
    idt_set_gate(14, (ULONG)isr14, 0x08, 0x8E);    idt_set_gate(15, (ULONG)isr15, 0x08, 0x8E);
    idt_set_gate(16, (ULONG)isr16, 0x08, 0x8E);    idt_set_gate(17, (ULONG)isr17, 0x08, 0x8E);
    idt_set_gate(18, (ULONG)isr18, 0x08, 0x8E);    idt_set_gate(19, (ULONG)isr19, 0x08, 0x8E);
    idt_set_gate(20, (ULONG)isr20, 0x08, 0x8E);    idt_set_gate(21, (ULONG)isr21, 0x08, 0x8E);
    idt_set_gate(22, (ULONG)isr22, 0x08, 0x8E);    idt_set_gate(23, (ULONG)isr23, 0x08, 0x8E);
    idt_set_gate(24, (ULONG)isr24, 0x08, 0x8E);    idt_set_gate(25, (ULONG)isr25, 0x08, 0x8E);
    idt_set_gate(26, (ULONG)isr26, 0x08, 0x8E);    idt_set_gate(27, (ULONG)isr27, 0x08, 0x8E);
    idt_set_gate(28, (ULONG)isr28, 0x08, 0x8E);    idt_set_gate(29, (ULONG)isr29, 0x08, 0x8E);
    idt_set_gate(30, (ULONG)isr30, 0x08, 0x8E);    idt_set_gate(31, (ULONG)isr31, 0x08, 0x8E);

    idt_set_gate(127,(ULONG)isr127,0x08, 0x8E); //Software interrupt for sys calls
}

/* Message string corresponding to the exception number 0-31: exception_messages[interrupt_number] */
char* exception_messages[] =
{
    "Division By Zero",        "Debug",                         "Non Maskable Interrupt",    "Breakpoint",
    "Into Detected Overflow",  "Out of Bounds",                 "Invalid Opcode",            "No Coprocessor",
    "Double Fault",            "Coprocessor Segment Overrun",   "Bad TSS",                   "Segment Not Present",
    "Stack Fault",             "General Protection Fault",      "Page Fault",                "Unknown Interrupt",
    "Coprocessor Fault",       "Alignment Check",               "Machine Check",             "Reserved",
    "Reserved",                "Reserved",                      "Reserved",                  "Reserved",
    "Reserved",                "Reserved",                      "Reserved",                  "Reserved",
    "Reserved",                "Reserved",                      "Reserved",                  "Reserved"
};

ULONG fault_handler(ULONG esp)
{
    ULONG retVal;
    struct regs* r = (struct regs*)esp;

    if(!pODA->ts_flag)
    {
        retVal = esp;
    }
    else
    {
        if(r->int_no == 127) //syscall
        {
            /// TODO: decision about task_switch
            retVal = esp;
            //retVal = task_switch(esp);
            /// TODO: decision about task_switch
            //printformat(" nr: %d esp: %x ", r->eax, esp);
        }
        else
        {
            retVal = task_switch(esp); //new task's esp
        }
    }

    if (r->int_no < 32) //exception
    {
        settextcolor(4,0);

        if (r->int_no == 6 || r->int_no == 1) //Invalid Opcode
        {
            printformat("err_code: %x address(eip): %x\n", r->err_code, r->eip);
            printformat("edi: %x esi: %x ebp: %x eax: %x ebx: %x ecx: %x edx: %x\n", r->edi, r->esi, r->ebp, r->eax, r->ebx, r->ecx, r->edx);
            printformat("cs: %x ds: %x es: %x fs: %x gs %x ss %x\n", r->cs, r->ds, r->es, r->fs, r->gs, r->ss);
            printformat("int_no %d eflags %x useresp %x\n", r->int_no, r->eflags, r->useresp);
        }

        if (r->int_no == 14) //Page Fault
        {
            ULONG faulting_address;
            asm volatile("mov %%cr2, %0" : "=r" (faulting_address)); // faulting address <== CR2 register

            // The error code gives us details of what happened.
            int present   = !(r->err_code & 0x1); // Page not present
            int rw        =   r->err_code & 0x2;  // Write operation?
            int us        =   r->err_code & 0x4;  // Processor was in user-mode?
            int reserved  =   r->err_code & 0x8;  // Overwritten CPU-reserved bits of page entry?
            int id        =   r->err_code & 0x10; // Caused by an instruction fetch?

            // Output an error message.
                          printformat("\nPage Fault (");
            if (present)  printformat("page not present");
            if (rw)       printformat(" read-only - write operation");
            if (us)       printformat(" user-mode");
            if (reserved) printformat(" overwritten CPU-reserved bits of page entry");
            if (id)       printformat(" caused by an instruction fetch");
                          printformat(") at %x - EIP: %x\n", faulting_address, r->eip);
        }

        /*
        printformat("TSS log:\n");
        TSS_log(&tss_entry);
        printformat("\n\n");
        */

        //log_task_list();
        //printformat("\n\n");

        printformat("err_code: %x address(eip): %x\n", r->err_code, r->eip);
        printformat("edi: %x esi: %x ebp: %x eax: %x ebx: %x ecx: %x edx: %x\n", r->edi, r->esi, r->ebp, r->eax, r->ebx, r->ecx, r->edx);
        printformat("cs: %x ds: %x es: %x fs: %x gs %x ss %x\n", r->cs, r->ds, r->es, r->fs, r->gs, r->ss);
        printformat("int_no %d eflags %x useresp %x\n", r->int_no, r->eflags, r->useresp);

        printformat("\n");
        printformat("%s >>> Exception. System Halted! <<<", exception_messages[r->int_no]);
        for (;;);
    }//if

    if (r->int_no == 127) //sw-interrupt syscall
    {
        syscall_handler(r);
    }
    return retVal;
}
