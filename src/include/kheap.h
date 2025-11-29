#ifndef KHEAP_H
#define KHEAP_H

#include "os.h"
#include "ordered_array.h"

#define KHEAP_START         0x40000000 // 1GB
#define KHEAP_INITIAL_SIZE  0x00300000
#define KHEAP_MAX           0x4FFFF000

#define HEAP_INDEX_SIZE     0x20000
#define HEAP_MAGIC          0x123890AB
#define HEAP_MIN_SIZE       0x70000

extern ULONG placement_address;

typedef struct
{
    ULONG magic;         // Magic number, used for error checking and identification.
    unsigned char is_hole;       // 1: hole, 0: block.
    ULONG size;          // size of the block, including the end footer.
} header_t;

typedef struct
{
    ULONG magic;         // Magic number, same as in header_t.
    header_t *header;    // Pointer to the block header.
} footer_t;

typedef struct
{
    ordered_array_t index;
    ULONG start_address; // The start of allocated space.
    ULONG end_address;   // The end of allocated space. May be expanded up to max_address.
    ULONG max_address;   // The maximum address the heap can be expanded to.
    unsigned char supervisor;    // Should extra pages requested by us be mapped as supervisor-only?
    unsigned char readonly;      // Should extra pages requested by us be mapped as read-only?
} heap_t;

heap_t* create_heap(ULONG start, ULONG end, ULONG max, unsigned char supervisor, unsigned char readonly);

void* alloc(ULONG size, unsigned char page_align, heap_t *heap);
void  free(void* p, heap_t* heap);
void  kfree(void* p);

#endif
