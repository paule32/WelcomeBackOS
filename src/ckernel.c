// ---------------------------------------------------------------------------
// \file kernel.c – A simple freestanding C-Kernel
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
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

extern ULONG placement_address;
extern heap_t* kheap;

extern const int DMA_BUFFER;

static void init()
{
    kheap = (void*)0;
    kernel_directory  = 0;
    current_directory = 0;
    placement_address = 0x200000;
}

int kmain()
{
    init();
    k_clear_screen();
    settextcolor(14,0);
    printformat("Back to 1985 OS [Version 1.0] (C) 2025 paule32\n");
    gdt_install();
    printformat("gdt     installed\n");
    idt_install();
    printformat("idt     installed\n");
    isrs_install();
    printformat("isr     installed\n");
    irq_install();
    printformat("irq     installed\n");
    initODA();
    printformat("ODA init\n");
    timer_install();
    printformat("timer   installed\n");
    keyboard_install();
    printformat("keybrd  installed\n");

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
           pODA->Memory_Size = 0x2000000; // 32 MB

        printformat("\nMemory detection does not work. Estimated usable RAM: %d KB", (pODA->Memory_Size)/1024);
    }
    printformat("\n\n");

    // paging, kernel heap, multitasking
    paging_install();
    printformat("paging  installed\n");
    //kheap = create_heap(KHEAP_START, KHEAP_START+KHEAP_INITIAL_SIZE, KHEAP_MAX, 1, 0); // SV and RW

    // RAM Disk
    ///
    #ifdef _DIAGNOSIS_
    settextcolor(2,0);
    printformat("rd_start: ");
    settextcolor(15,0);
    #endif
    ///
    ULONG ramdisk_start = k_malloc(0x200000, 0, 0);
    settextcolor(15,0);
    printformat("Ram Disk at: %x\n",ramdisk_start);

    tasking_install(); // ends with sti()
    printformat("tasking installed\n");

    /// //////////////////////// TEST INSTALL FLOPPY DRIVER
	printformat("\nFloppy Driver Test!\n\n");
	flpydsk_set_working_drive(0); // set drive 0 as current drive
	flpydsk_install(6);           // floppy disk uses IRQ 6
	/// //////////////////////// TEST INSTALL FLOPPY DRIVER
    printformat("floppy installed\n");

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
for(;;);
    // search the content of files <- data from outside "loaded" via incbin ...
    settextcolor(15,0);
    i=0;
    
    struct dirent* node = 0;
    
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
            printformat("%d \t%s\n",sz,name);

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
            }
        }
        ++i;
    }
    printformat("\n");
for(;;);
        // shell in elf-executable-format provided by data.asm
    ULONG elf_vaddr     = *( (ULONG*)( address_TEST + 0x3C ) );
    ULONG elf_offset    = *( (ULONG*)( address_TEST + 0x38 ) );
    ULONG elf_filesize  = *( (ULONG*)( address_TEST + 0x44 ) );

    ///
    #ifdef _DIAGNOSIS_
    settextcolor(5,0);
    printformat("virtual address: %x offset: %x size: %x", elf_vaddr, elf_offset, elf_filesize);
    printformat("\n\n");
    settextcolor(15,0);
    #endif
    ///

    if( (elf_vaddr>0x5FF000) || (elf_vaddr<0x400000) || (elf_offset>0x130) || (elf_filesize>0x1000) )
    {
        flag1=2;
    }

    // Test-Programm ==> user space
    if(flag1==1)
    {
        address_user = elf_vaddr;
        k_memcpy((void*)address_user, address_TEST + elf_offset, elf_filesize); // ELF LOAD: Offset VADDR FileSize
    }

    pODA->ts_flag = 1; // enable task_switching

    if(flag1==1)
    {
      create_task ((void*)getAddressUserProgram(),3); // program in user space (ring 3) takes over
    }
    else
    {
        if(flag1==0)
        {
            settextcolor(4,0);
            printformat("Program not found.\n");
            settextcolor(15,0);
        }
        else
        {
            settextcolor(4,0);
            printformat("Program corrupt.\n");
            settextcolor(15,0);
        }
    }

    while(TRUE){/*  */}
    return 0;
}
