// ---------------------------------------------------------------------------
// \file  kernel.c – A simple freestanding C-Kernel
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
# include "config.h"

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

# define DESKTOP
# include "vga.h"

extern void* krealloc(void* ptr, uint32_t new_size);

extern int atapi_read_sectors(uint32_t lba, uint32_t count, void *buffer);
extern int  sata_read_sectors(uint32_t lba, uint32_t count, void *buffer);

extern "C" void printformat(char*, ...);
extern "C" void detect_memory(void);
extern "C" void enter_usermode(void);
extern "C" void irq_init(void);
extern "C" void idt_init(void);
extern "C" void isr_init(void);
extern "C" void gdt_init();
extern "C" void syscall_init(void);
extern "C" void tasking_init(void);

extern "C" void call_global_ctors(void);

extern "C" void* __gxx_personality_v0(){return (void*)0;}
int  graph_mode = 0;

extern uint32_t __end;

# define KSTACK_SIZE  (64*1024)

uint32_t kernel_stack_top    = 0x00180000;
uint32_t kernel_stack_bottom = kernel_stack_top - KSTACK_SIZE;

// text kernel
extern "C" int txt_main()
{
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    vga[0] = 0x0F00 | 'K';
    graph_mode = 1;

    idt_init();
 
    paging_init();
    kheap_init();

    //detect_memory();          // setzt max_mem
    
    uint32_t stack_top = kernel_stack_top;
    uint32_t stack_bottom = stack_top - (64*1024);

    uint32_t reserved = ((uint32_t)&__end + 0xFFF) & ~0xFFF;
    if (reserved < kernel_stack_top) reserved = kernel_stack_top; // Stack schützen
    page_init(reserved);
    
    // 3) aktuellen Stack als esp0 für TSS verwenden
    uint32_t esp;
    __asm__ volatile("mov %%esp, %0" : "=r"(esp));
    gdt_init();

    isr_init();
    irq_init();

    syscall_init();
    tasking_init();

    // ------------------------------
    // global c++ constructor's init.
    // ------------------------------
    call_global_ctors();

    settextcolor(14,0);
    
    if (check_atapi() == 0) {
        // ATAPI (IDE) gefunden
        printformat("ATAPI: OK.\n");
    }   else {
        printformat("ATAPI: NOK\n");
        check_ahci();
    }

    if (iso_mount() != 0) {
        printformat("ISO mount Error.\n");
    }   else {
        printformat("ISO mount successfully.\n");
    }

    __asm__ volatile("sti");
    
    // Testmarker, bevor wir springen:
    volatile char* VGA = (volatile char*)0xB8000;
    VGA[0] = 'K'; VGA[1] = 0x0F;
    VGA[2] = 'U'; VGA[3] = 0x0F;

    enter_usermode();
    
    // Hierher kommt man normalerweise nicht mehr zurück
    for (;;) {
        asm volatile("hlt");
    }
    return 0;

    printformat("\nin text kernel\n");
    for (;;);
}

// graphics kernel
extern "C" int vid_main()
{
    graph_mode = 2;
    
    idt_init();
 
    paging_init();
    kheap_init();
 
    //detect_memory();          // setzt max_mem
    
    uint32_t stack_top = kernel_stack_top;
    uint32_t stack_bottom = stack_top - (64*1024);

    uint32_t reserved = ((uint32_t)&__end + 0xFFF) & ~0xFFF;
    if (reserved < kernel_stack_top) reserved = kernel_stack_top; // Stack schützen
    page_init(reserved);

    // 3) aktuellen Stack als esp0 für TSS verwenden
    uint32_t esp;
    //__asm__ volatile("mov %%esp, %0" : "=r"(esp));
    gdt_init();
    isr_init();
    irq_init();
    
    syscall_init();
    tasking_init();
    
    // ------------------------------
    // vesa 0x114: 800 x 600 x 16bpp
    // ------------------------------
    gfx_init();
    
    // ------------------------------
    // global c++ constructor's init.
    // ------------------------------
    call_global_ctors();
    
    //try {
        //gfx_printf("ctorflag = %d\n", 123);
        /*extern TCanvas canvas_desktop;
        extern "C" volatile int g_ctor_ran;
        gfx_printf("ctorflag=%d\n", g_ctor_ran);*/
    //}
    //catch (...) {
//        gfx_printf("xxxxxx\n");
//        for(;;);
//    }
    
    
    /*TCanvas *foo = new TCanvas();
    if (canvas_desktop.flag == 42) {
        for (;;);
    }*/
    
    settextcolor(14,0);
    
    if (check_atapi() == 0) {
        // ATAPI (IDE) gefunden
        gfx_printf("ATAPI: OK.\n");
    }   else {
        gfx_printf("ATAPI: NO\n");
        check_ahci();
    }

    if (iso_mount() != 0) {
        gfx_printf("ISO mount Error.\n");
    }   else {
        gfx_printf("ISO mount successfully.\n");
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
