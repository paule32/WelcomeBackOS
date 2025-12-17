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

extern int atapi_read_sectors(uint32_t lba, uint32_t count, void *buffer);
extern int  sata_read_sectors(uint32_t lba, uint32_t count, void *buffer);

extern void printformat(char*, ...);
extern void detect_memory(void);
extern void enter_usermode(void);

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
    
#if 0
    task_create(test_task);
    
    asm volatile("sti");
    
    //vga[4] = 'P';
    //vga[5] = 0x0F;
    
    char* p = (char*)kmalloc(32);
    if (p) {
        p[0] = 'O';
        p[1] = 'K';
        p[2] = 0;
        vga[6] = p[0]; vga[7] = 0x1F;
        vga[8] = p[1]; vga[9] = 0x1F;
    }   else {
        vga[ 6] = 'E'; vga[ 7] = 0x21;
        vga[ 8] = 'R'; vga[ 9] = 0x21;
        vga[10] = 'R'; vga[11] = 0x21;
    }
    
    p = (char*)krealloc(p, 128);
    if (p) {
        // Realloc ok
        vga[12] = 'O'; vga[13] = 0x21;
        vga[14] = 'L'; vga[15] = 0x21;
    }   else {
        // Realloc failed
        vga[12] = 'X'; vga[13] = 0x02;
    }
    
    // Test: Syscall direkt aus Kernel ( später: aus Usermode )
    asm volatile("mov $1, %eax");   // SYSCALL_PUTCHAR
    asm volatile("mov $'@', %ebx");
    asm volatile("int $0x80");
#endif
    
    // Hierher kommt man normalerweise nicht mehr zurück
    for (;;) {
        asm volatile("hlt");
    }
    return 0;
}


static void putc_xy(int x, int y, char c, uint8_t color)
{
    static volatile char* const VGA = (volatile char*)0xB8000;
    int pos = y * 80 + x;
    
    VGA[pos * 2]     = c;
    VGA[pos * 2 + 1] = color;
}

void test_task(void)
{
    const char* msg = "Task2 running...";
    int x = 0;
    int y = 3;
    int idx = 0;

    while (1) {
        putc_xy(x + (idx % 20), y, msg[idx % 16], 0x0E);
        idx++;

        // kleine Delay-Schleife, damit sich was tut
        for (volatile uint32_t i = 0; i < 1000000; ++i) {
            // busy-wait
        }
    }
}

#if 0
#include "os.h"
#include "kheap.h"
#include "task.h"
#include "initrd.h"
#include "syscall.h"
#include "shared_pages.h"
#include "flpydsk.h"

// RAM Detection by Second Stage Bootloader
#define ADDR_MEM_ENTRIES 0x0FFE
#define ADDR_MEM_INFO    0x1000

// Buffer for User-Space Program
#define FILEBUFFERSIZE   0x2000

extern page_directory_t* kernel_directory;
extern page_directory_t* current_directory;
extern ULONG file_data_start __asm__("_file_data_start");
extern ULONG file_data_end   __asm__("_file_data_end"  );
ULONG address_user;
unsigned char address_TEST[FILEBUFFERSIZE];
unsigned char buf[FILEBUFFERSIZE];
unsigned char flag1 = 0; // status of user-space-program
Mem_Chunk_t Mem_Chunk[10]; // contiguous parts of memory detected by int 15h eax = 820h

extern heap_t* kheap;

extern const int DMA_BUFFER;

extern void vbe_read_modeinfo_early(void);
extern void vbe_init_pm(void);

extern void put_pixel(USHORT, USHORT, USHORT);
extern USHORT rgb565(UCHAR, UCHAR, UCHAR);
extern void user_program_1(void);
extern void* _end;

static void init()
{
    kheap = (void*)0;
    kernel_directory  = 0;
    current_directory = 0;
    placement_address = (ULONG)&_end;
}

int kmain()
{
    // --------------------------------------
    // mother of all is the graphics card ...
    // --------------------------------------
    //vbe_init_pm();
    init();
    
    k_clear_screen();
    settextcolor(14,0);
    printformat("Back to 1985 OS [Version 1.0] (C) 2025 paule32\n");
    gdt_install();
    idt_install();
    isrs_install();
    irq_install();
    initODA();
    timer_install();
    keyboard_install();

    // get physical memory which is usable RAM
    USHORT num_of_entries = *( (USHORT*)(ADDR_MEM_ENTRIES) );

    //#ifdef _DIAGNOSIS_
    settextcolor(2,0);
    printformat("\nNUM of RAM-Entries: %d", num_of_entries);
    settextcolor(15,0);
    //#endif

    pODA->Memory_Size = 0;
    ULONG i,j;
    for(i=0; i<num_of_entries; ++i)
    {
        for(j=0; j<24; j+=4)
        {
            if(j== 0) Mem_Chunk[i].base_lo   = *( (ULONG*)(ADDR_MEM_INFO+i*24+j) );
            if(j== 4) Mem_Chunk[i].base_hi   = *( (ULONG*)(ADDR_MEM_INFO+i*24+j) );
            if(j== 8) Mem_Chunk[i].length_lo = *( (ULONG*)(ADDR_MEM_INFO+i*24+j) );
            if(j==12) Mem_Chunk[i].length_hi = *( (ULONG*)(ADDR_MEM_INFO+i*24+j) );
            if(j==16) Mem_Chunk[i].type      = *( (ULONG*)(ADDR_MEM_INFO+i*24+j) );
            if(j==20) Mem_Chunk[i].extended  = *( (ULONG*)(ADDR_MEM_INFO+i*24+j) );
        }
        if((Mem_Chunk[i].type)==1) pODA->Memory_Size += Mem_Chunk[i].length_lo;
    }
    if( (pODA->Memory_Size>0)&&(pODA->Memory_Size<=0xFFFFFFFF) )
    {
        printformat("\nUsable RAM: %d KB", (pODA->Memory_Size)/1024);
    }
    else
    {
        if(pODA->Memory_Size==0)
           pODA->Memory_Size = 0x4000000; // 32 MB

        printformat("\nMemory detection does not work. Estimated usable RAM: %d KB", (pODA->Memory_Size)/1024);
    }
    printformat("\n\n");

    // paging, kernel heap, multitasking
    //paging_install();

    //kheap = create_heap(KHEAP_START, KHEAP_START+KHEAP_INITIAL_SIZE, KHEAP_MAX, 1, 0); // SV and RW

    // RAM Disk
    ///
    #ifdef _DIAGNOSIS_
    settextcolor(2,0);
    printformat("rd_start: ");
    settextcolor(15,0);
    #endif
    ///
    ULONG ramdisk_start = k_malloc(0x400000, 0, 0);
    settextcolor(15,0);
    printformat("Ram Disk at: %x\n",ramdisk_start);

    tasking_install(); // ends with sti()
    
    /// //////////////////////// TEST INSTALL FLOPPY DRIVER
	printformat("\nFloppy Driver Test!\n\n");
	flpydsk_set_working_drive(0); // set drive 0 as current drive
	flpydsk_install(6);           // floppy disk uses IRQ 6
	/// //////////////////////// TEST INSTALL FLOPPY DRIVER
    
	/// //////////////////////// TEST FLOPPY DRIVER READ DIRECTORY
	k_memset((void*)DMA_BUFFER, 0x0, 0x2400);
	flpydsk_read_sector(19); // start at 0x2600: root directory (14 sectors)
	printformat("<Floppy Disc Root Dir>\n");
	for(i=0;i<224;++i)       // 224 Entries * 32 Byte
	{
        if(
			(( *((unsigned char*)(DMA_BUFFER + i*32)) )      != 0x00 ) && /* free from here on           */
			(( *((unsigned char*)(DMA_BUFFER + i*32)) )      != 0xE5 ) && /* 0xE5 deleted = free         */
			(( *((unsigned char*)(DMA_BUFFER + i*32 + 11)) ) != 0x0F )    /* 0x0F part of long file name */
		  )
		{
			int start = DMA_BUFFER + i*32; // name
			int count = 8;
			char* end = (char*)(start+count);
			for(; count != 0; --count)
			{
			    if( *(end-count) != 0x20 ) /* empty space in file name */
				    printformat("%c",*(end-count));
			}

			if(i!=0) printformat("."); // usual separator between file name and file extension

			start = DMA_BUFFER + i*32 + 8; // extension
			count = 3;
			end = (char*)(start+count);
			for(; count != 0; --count)
				printformat("%c",*(end-count));
			printformat("\t%d byte", *((ULONG*)(DMA_BUFFER + i*32 + 28)));
			printformat("\t");
			if((( *((unsigned char*)(DMA_BUFFER + i*32 + 11)) ) & 0x08 ) == 0x08 ) printformat(" (lab)");
			if((( *((unsigned char*)(DMA_BUFFER + i*32 + 11)) ) & 0x10 ) == 0x10 ) printformat(" (dir)");
			if((( *((unsigned char*)(DMA_BUFFER + i*32 + 11)) ) & 0x01 ) == 0x01 ) printformat(" (r/o)");
			if((( *((unsigned char*)(DMA_BUFFER + i*32 + 11)) ) & 0x02 ) == 0x02 ) printformat(" (hid)");
			if((( *((unsigned char*)(DMA_BUFFER + i*32 + 11)) ) & 0x04 ) == 0x04 ) printformat(" (sys)");
			if((( *((unsigned char*)(DMA_BUFFER + i*32 + 11)) ) & 0x20 ) == 0x20 ) printformat(" (arc)");
			printformat("\n");
		}
	}
    printformat("\n");
	/// //////////////////////// TEST FLOPPY DRIVER READ DIRECTORY

    // test with data and program from data.asm
    k_memcpy((void*)ramdisk_start, &file_data_start, (ULONG)&file_data_end - (ULONG)&file_data_start);
    
    fs_root = install_initrd(ramdisk_start);
    printformat("init ramdisk\n");

    // search the content of files <- data from outside "loaded" via incbin ...
    settextcolor(15,0);
    i=0;

    struct dirent* node = 0;
    
    #if 0
    node = readdir_fs(fs_root, 0);
    fs_node_t* fsnode = finddir_fs(fs_root, node->name);
    printformat("===>  %s\n", node->name);
    ULONG sz = read_fs(fsnode, 0, 10, buf);
    printformat("oo> %s\n", buf);
    #endif

    
    while( (node = readdir_fs(fs_root, i)) != 0)
    {
        fs_node_t* fsnode = finddir_fs(fs_root, node->name);

        if((fsnode->flags & 0x7) == FS_DIRECTORY)
        {
            printformat("<DIR> %s\n",node->name);
        }
        else
        {
            ULONG sz = read_fs(fsnode, 0, fsnode->length, buf);

            char name[40];
            k_memset((void*)name, 0, 40);
            k_memcpy((void*)name, node->name, 35); // protection against wrong / too long filename
            //printformat("%d \t%s\n",sz,name);

            ULONG j;
            if( k_strcmp(node->name,"shell")==0 )
            {
                flag1=1;

                if(sz>=FILEBUFFERSIZE)
                {
                    sz = 0;
                    settextcolor(4,0);
                    printformat("Buffer Overflow prohibited!\n");
                    settextcolor(15,0);
                }

                for(j=0; j<sz; ++j)
                    address_TEST[j] = buf[j];
            }   else {
                printformat(" buffer: %s", buf);
            }
        }
        ++i;
    }

    user_program_1();
    return 0;
}
#endif
