#include "paging.h"
#include "kheap.h"
#include "vbe.h"

ULONG ULONG_MAX = 0xFFFFFFFF;
page_directory_t* kernel_directory  = 0;
page_directory_t* current_directory = 0;

ULONG placement_address = 0x200000;
extern heap_t* kheap;
extern ULONG lfb_phys;

// A bitset of frames - used or free
ULONG*  frames; // pointer to the bitset (functions: set/clear/test)

ULONG k_malloc(ULONG size, unsigned char align, ULONG* phys)
{
    if( kheap!=0 )
    {
        ULONG addr = (ULONG) alloc(size, align, kheap);
        if (phys != 0)
        {
            page_t* page = get_page(addr, 0, kernel_directory);
            *phys = page->frame * PAGESIZE + (addr&0xFFF);
        }

        ///
        #ifdef _DIAGNOSIS_
        settextcolor(3,0);
        printformat("%x ",addr);
        settextcolor(15,0);
        #endif
        ///

        return addr;
    }
    else
    {
        if( !(placement_address == (placement_address & 0xFFFFF000) ) )
        {
            placement_address &= 0xFFFFF000;
            placement_address += PAGESIZE;
        }

        if( phys )
        {
            *phys = placement_address;
        }
        ULONG temp = placement_address;
        placement_address += size;     // new placement_address is increased

        ///
        #ifdef _DIAGNOSIS_
        settextcolor(9,0);
        printformat("%x ",temp);
        settextcolor(15,0);
        #endif
        ///

        return temp;                   // old placement_address is returned
    }
}

/************* bitset variables and functions **************/
ULONG ind, offs;

static void get_Index_and_Offset(ULONG frame_addr)
{
    ULONG frame    = frame_addr/PAGESIZE;
    ind    = frame/32;
    offs   = frame%32;
}

static void set_frame(ULONG frame_addr)
{
    get_Index_and_Offset(frame_addr);
    frames[ind] |= (1<<offs);
}

static void clear_frame(ULONG frame_addr)
{
    get_Index_and_Offset(frame_addr);
    frames[ind] &= ~(1<<offs);
}

/*
static ULONG test_frame(ULONG frame_addr)
{
    get_Index_and_Offset(frame_addr);
    return( frames[ind] & (1<<offs) );
}
*/
/***********************************************************/

static ULONG first_frame() // find the first free frame in frames bitset
{
    ULONG index, offset;
    for(index=0; index<( (pODA->Memory_Size/PAGESIZE)/32 ); ++index)
    {
        if(frames[index] != ULONG_MAX)
        {
            for(offset=0; offset<32; ++offset)
            {
                if( !(frames[index] & 1<<offset) ) // bit set to zero?
                    return (index*32 + offset);
            }
        }
    }
    return ULONG_MAX; // no free page frames
}

void alloc_frame(page_t* page, int is_kernel, int is_writeable) // allocate a frame
{
    if( !(page->frame) )
    {
        ULONG index = first_frame(); // search first free page frame
        if( index == ULONG_MAX )
            printformat("message from alloc_frame: no free frames!!! ");

        set_frame(index*PAGESIZE);

        page->present    = 1;
        page->rw         = ( is_writeable == 1 ) ? 1 : 0;
        page->user       = ( is_kernel    == 1 ) ? 0 : 1;
        page->frame      = index;
    }
}

void free_frame(page_t* page) // deallocate a frame
{
    if( page->frame )
    {
        clear_frame(page->frame);
        page->frame = 0;
    }
}

void paging_install()
{
    // setup bitset
    ///
    #ifdef _DIAGNOSIS_
    settextcolor(2,0);
    printformat("bitset: ");
    settextcolor(15,0);
    #endif
    ///

    frames = (ULONG*) k_malloc( (pODA->Memory_Size/PAGESIZE) /32, 0, 0 );
    k_memset( frames, 0, (pODA->Memory_Size/PAGESIZE) /32 );

    // make a kernel page directory
    ///
    #ifdef _DIAGNOSIS_
    settextcolor(2,0);
    printformat("PD: ");
    settextcolor(15,0);
    #endif
    ///

    kernel_directory = (page_directory_t*) k_malloc( sizeof(page_directory_t), 1, 0 );
    k_memset(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physicalAddr = (ULONG)kernel_directory->tablesPhysical;

    ULONG i=0;
    // Map some pages in the kernel heap area.
    for( i=KHEAP_START; i<KHEAP_START+KHEAP_INITIAL_SIZE; i+=PAGESIZE )
        get_page(i, 1, kernel_directory);

    // map (phys addr <---> virt addr) from 0x0 to the end of used memory
    // Allocate at least 0x2000 extra, that the kernel heap, tasks, and kernel stacks can be initialized properly
    i=0;
    while( i < placement_address + 0x6000 ) //important to add more!
    {
        if( ((i>=0xb8000) && (i<=0xbf000)) || ((i>=0x17000) && (i<0x18000)) )
        {
            alloc_frame( get_page(i, 1, kernel_directory), US, RW); // user and read-write
        }
        else
        {
            alloc_frame( get_page(i, 1, kernel_directory), SV, RO); // supervisor and read-only
        }
        i += PAGESIZE;
    }
    
    // 2) LFB zusätzlich identity-mappen (nur, wenn vorhanden)
    if (lfb_phys != 0)
    {
        ULONG addr = lfb_phys & 0xFFFFF000;        // auf 4K-Grenze ausrichten
        ULONG end  = addr + 0x400000;              // z.B. 4MB für den Framebuffer

        while (addr < end)
        {
            page_t* page = get_page(addr, 1, kernel_directory);
            page->present    = 1;
            page->rw         = 1;
            page->user       = 0;             // Kernel
            page->frame      = addr >> 12;    // physische Frame-Nummer

            addr += PAGESIZE;
        }
    }

    //Allocate user space
    ULONG user_space_start = 0x400000;
    ULONG user_space_end   = 0x600000;
    i=user_space_start;
    while( i <= user_space_end )
    {
        alloc_frame( get_page(i, 1, kernel_directory), US, RW); // user and read-write
        i += PAGESIZE;
    }

    // Now allocate those pages we mapped earlier.
    for( i=KHEAP_START; i<KHEAP_START+KHEAP_INITIAL_SIZE; i+=PAGESIZE )
         alloc_frame( get_page(i, 1, kernel_directory), SV, RW); // supervisor and read/write

    current_directory = clone_directory(kernel_directory);

    // cr3: PDBR (Page Directory Base Register)
    asm volatile("mov %0, %%cr3":: "r"(kernel_directory->physicalAddr)); //set page directory base pointer

    // read cr0, set paging bit, write cr0 back
    ULONG cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0)); // read cr0
    cr0 |= 0x80000000; // set the paging bit in CR0 to enable paging
    asm volatile("mov %0, %%cr0":: "r"(cr0)); // write cr0
}

page_t* get_page(ULONG address, unsigned char make, page_directory_t* dir)
{
    address /= PAGESIZE;                // address ==> index.
    ULONG table_index = address / 1024; // ==> page table containing this address

    if(dir->tables[table_index])       // table already assigned
    {
        return &dir->tables[table_index]->pages[address%1024];
    }
    else if(make)
    {
        ULONG phys;
        ///
        #ifdef _DIAGNOSIS_
        settextcolor(2,0);
        printformat("gp_make: ");
        settextcolor(15,0);
        #endif
        ///

        dir->tables[table_index] = (page_table_t*) k_malloc( sizeof(page_table_t), 1, &phys );
        k_memset(dir->tables[table_index], 0, PAGESIZE);
        dir->tablesPhysical[table_index] = phys | 0x7; // 111b meaning: PRESENT=1, RW=1, USER=1
        return &dir->tables[table_index]->pages[address%1024];
    }
    else
        return 0;
}

static page_table_t *clone_table(page_table_t* src, ULONG* physAddr)
{
    // Make a new page table, which is page aligned.
    ///
    #ifdef _DIAGNOSIS_
    settextcolor(2,0);
    printformat("clone_PT: ");
    settextcolor(15,0);
    #endif
    ///

    page_table_t *table = (page_table_t*)k_malloc(sizeof(page_table_t),1,physAddr);
    // Ensure that the new table is blank.
    k_memset(table, 0, sizeof(page_directory_t));

    // For every entry in the table...
    int i;
    for(i=0; i<1024; ++i)
    {
        // If the source entry has a frame associated with it...
        if (!src->pages[i].frame)
            continue;
        // Get a new frame.
        alloc_frame(&table->pages[i], 0, 0);
        // Clone the flags from source to destination.
        if (src->pages[i].present) table->pages[i].present = 1;
        if (src->pages[i].rw)      table->pages[i].rw = 1;
        if (src->pages[i].user)    table->pages[i].user = 1;
        if (src->pages[i].accessed)table->pages[i].accessed = 1;
        if (src->pages[i].dirty)   table->pages[i].dirty = 1;

        // Physically copy the data across. This function is in process.s.
        copy_page_physical(src->pages[i].frame*0x1000, table->pages[i].frame*0x1000);
    }
    return table;
}

page_directory_t *clone_directory(page_directory_t *src)
{
    ULONG phys;
    // Make a new page directory and obtain its physical address.
    ///
    #ifdef _DIAGNOSIS_
    settextcolor(2,0);
    printformat("clone_PD: ");
    settextcolor(15,0);
    #endif
    ///

    page_directory_t *dir = (page_directory_t*) k_malloc( sizeof(page_directory_t),1,&phys );
    // Ensure that it is blank.
    k_memset( dir, 0, sizeof(page_directory_t) );

    // Get the offset of tablesPhysical from the start of the page_directory_t structure.
    ULONG offset = (ULONG)dir->tablesPhysical - (ULONG)dir;

    // Then the physical address of dir->tablesPhysical is:
    dir->physicalAddr = phys + offset;

    // Go through each page table. If the page table is in the kernel directory, do not make a new copy.
    int i;
    for(i=0; i<1024; ++i)
    {
        if (!src->tables[i])
            continue;

        if (kernel_directory->tables[i] == src->tables[i])
        {
            // It's in the kernel, so just use the same pointer.
            dir->tables[i] = src->tables[i];
            dir->tablesPhysical[i] = src->tablesPhysical[i];
        }
        else
        {
            // Copy the table.
            ULONG phys;
            dir->tables[i] = clone_table(src->tables[i], &phys);
            dir->tablesPhysical[i] = phys | 0x07;
        }
    }
    return dir;
}

void analyze_frames_bitset(ULONG sec)
{
    ULONG index, offset, counter1=0, counter2=0;
    for(index=0; index<( (pODA->Memory_Size/PAGESIZE) /32); ++index)
    {
        settextcolor(15,0);
        printformat("\n%x  ",index*32*0x1000);
        ++counter1;
        for(offset=0; offset<32; ++offset)
        {
            if( !(frames[index] & 1<<offset) )
            {
                settextcolor(4,0);
                putch('0');
            }
            else
            {
                settextcolor(2,0);
                putch('1');
                ++counter2;
            }
        }
        if(counter1==24)
        {
            counter1=0;
            if(counter2)
                sleepSeconds(sec);
            counter2=0;
        }
    }
}

ULONG show_physical_address(ULONG virtual_address)
{
    page_t* page = get_page(virtual_address, 0, kernel_directory);
    return( (page->frame)*PAGESIZE + (virtual_address&0xFFF) );
}

void analyze_physical_addresses()
{
    int i,j,k=0, k_old;
    for(i=0; i<( (pODA->Memory_Size/PAGESIZE) / 0x18000 + 1 ); ++i)
    {
        for(j=i*0x18000; j<i*0x18000+0x18000; j+=0x1000)
        {
            if(show_physical_address(j)==0)
            {
                settextcolor(4,0);
                k_old=k; k=1;
            }
            else
            {
                if(show_physical_address(j)-j)
                {
                    settextcolor(3,0);
                    k_old=k; k=2;
                }
                else
                {
                    settextcolor(2,0);
                    k_old=k; k=3;
                }
            }
            if(k!=k_old)
                printformat("%x %x\n", j, show_physical_address(j));
        }
    }
}
