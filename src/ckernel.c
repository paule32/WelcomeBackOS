// ---------------------------------------------------------------------------
// \file  kernel.c – A simple freestanding C-Kernel
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
# include "stdint.h"
# include "paging.h"
# include "kheap.h"
# include "gdt.h"
# include "idt.h"
# include "isr.h"
# include "task.h"
# include "syscall.h"
# include "usermode.h"
# include "iso9660.h"
# include "proto.h"

extern void* krealloc(void* ptr, uint32_t new_size);
extern void gdt_init(uint32_t);
extern void irq_init(void);
extern int  gfx_init(void);

extern int atapi_read_sectors(uint32_t lba, uint32_t count, void *buffer);
extern int  sata_read_sectors(uint32_t lba, uint32_t count, void *buffer);

extern void printformat(char*, ...);
extern void detect_memory(void);
extern void enter_usermode(void);

extern uint32_t __end;

void test_task(void);

uint32_t kernel_stack_top = 0x00180000;

int kmain()
{
    /*
    volatile char *vga = (volatile char*)0xB8000;
    vga[2] = 'M';
    vga[3] = 0x0F;*/
    
    paging_init();
    kheap_init();
    
    detect_memory();          // setzt max_mem
    uint32_t reserved = (uint32_t)(&__end) + 0x00100000;
    page_init(reserved);
    
    // 3) aktuellen Stack als esp0 für TSS verwenden
    uint32_t esp;
    __asm__ volatile("mov %%esp, %0" : "=r"(esp));
    gdt_init(esp);
    idt_init();
    isr_init();
    irq_init();
    
    syscall_init();
    tasking_init();
    
    gfx_init();
    
    settextcolor(14,0);
    
    if (check_atapi() == 0) {
        // ATAPI (IDE) gefunden
        printformat("OK ATAPI\n");
    }   else {
        printformat("NO ATAPI\n");
        check_ahci();
    }

    if (iso_mount() != 0) {
        printformat("ISO mount Error.\n");
    }   else {
        printformat("ISO mount successfully.\n");
    }

    __asm__ volatile("sti");
    
    // Testmarker, bevor wir springen:
    /*volatile char* VGA = (volatile char*)0xB8000;
    VGA[0] = 'K'; VGA[1] = 0x0F;
    VGA[2] = 'U'; VGA[3] = 0x0F;*/

    enter_usermode();
    
    // Hierher kommt man normalerweise nicht mehr zurück
    for (;;) {
        asm volatile("hlt");
    }
    return 0;
}
