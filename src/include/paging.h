#ifndef PAGING_H
#define PAGING_H

#include "os.h"

#define PAGESIZE        0x00001000           // 0x1000 = 4096 = 4K

#define SV   1   // supervisor
#define US   0   // user

#define RW   1   // read-write
#define RO   0   // read-only

/************************************************************************/
// page_directory ==> page_table ==> page

typedef struct page
{
    ULONG present    :  1;   // Page present in memory
    ULONG rw         :  1;   // 0: read-only,    1: read/write
    ULONG user       :  1;   // 0: kernel level, 1: user level
    ULONG accessed   :  1;   // Has the page been accessed since last refresh?
    ULONG dirty      :  1;   // Has the page been written to since last refresh?
    ULONG unused     :  7;   // Combination of unused and reserved bits
    ULONG frame_addr : 20;   // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table
{
    page_t pages[1024];
} page_table_t;

typedef struct page_directory
{
    page_table_t* tables[1024];
    ULONG tablesPhysical[1024];
    ULONG physicalAddr;
} page_directory_t;
/************************************************************************/

void              paging_install();

page_t*           get_page(ULONG address, unsigned char make, page_directory_t* dir);

void              alloc_frame(page_t *page, int is_kernel, int is_writeable);
void              free_frame (page_t *page);

ULONG             k_malloc(ULONG sz, unsigned char align, ULONG* phys);
ULONG             k_malloc_stack(ULONG sz, unsigned char align, ULONG* phys);

page_directory_t* clone_directory(page_directory_t *src);

ULONG show_physical_address(ULONG virtual_address);

extern void       copy_page_physical(ULONG source_address, ULONG dest_address); //process.asm

#endif
