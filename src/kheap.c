// kheap.c
// Very simple first-fit kernel heap
// (c) 2025 by Jens Kallup - paule32

# include "stdint.h"
# include "kheap.h"

// Symbol from linker script: first free byte after kernel + bss
extern uint8_t _end;

// 1 MiB heap size (can be adjusted)
#define KHEAP_SIZE   (0x00100000)   // 1 MiB

typedef struct heap_block {
    uint32_t size;              // size of the usable area (in bytes)
    uint8_t  free;              // 1 = free, 0 = used
    uint8_t  _pad[3];           // padding for alignment
    struct heap_block* next;    // next block in list
}   heap_block_t;

static heap_block_t* heap_head = NULL;

// simple helper: align up to 'align' (must be power of 2)
static inline uint32_t align_up(uint32_t value, uint32_t align)
{
    return (value + align - 1) & ~(align - 1);
}

// simple memcpy, weil keine libc vorhanden ist
void* kmemcpy(void* dst, const void* src, uint32_t n)
{
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;

    for (uint32_t i = 0; i < n; ++i)
    d[i] = s[i];

    return dst;
}

void kheap_init(void)
{
    // Start of heap directly after the kernel
    uint32_t start = (uint32_t)&_end;

    // Align heap start to 8 bytes for sanity
    start = align_up((uint32_t)start, 8);

    heap_head = (heap_block_t*)start;

    // total heap space = KHEAP_SIZE minus header
    heap_head->size = KHEAP_SIZE - sizeof(heap_block_t);
    heap_head->free = 1;
    heap_head->next = NULL;
}

// Split a block into "used block" + "remainder" if large enough
static void split_block(heap_block_t* block, uint32_t size)
{
    // ensure we have enough space for a new header + some payload
    if (block->size >= size + sizeof(heap_block_t) + 16) {
        uintptr_t new_addr = (uintptr_t)block + sizeof(heap_block_t) + size;
        heap_block_t* new_block = (heap_block_t*)new_addr;

        new_block->size = block->size - size - sizeof(heap_block_t);
        new_block->free = 1;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;
    }
}

// Merge adjacent free blocks
static void merge_free_blocks(void)
{
    heap_block_t* current = heap_head;
    while (current && current->next) {
        if (current->free && current->next->free) {
            // merge next into current
            current->size += sizeof(heap_block_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void* kmalloc(uint32_t size)
{
    if (size == 0) return NULL;

    // align requested size to 8 bytes
    size = align_up(size, 8);

    heap_block_t* current = heap_head;

    while (current) {
        if (current->free && current->size >= size) {
            // block fits
            split_block(current, size);
            current->free = 0;

            // return pointer directly after header
            return (void*)((uintptr_t)current + sizeof(heap_block_t));
        }
        current = current->next;
    }

    // no suitable free block found
    return NULL;
}

void kfree(void* ptr)
{
    if (!ptr) return;

    // get header
    heap_block_t* block =
    (heap_block_t*)((uintptr_t)ptr - sizeof(heap_block_t));

    block->free = 1;

    // try to merge neighboring free blocks
    merge_free_blocks();
}

void* krealloc(void* ptr, uint32_t new_size)
{
    if (ptr == NULL) {
        // behaves like malloc
        return kmalloc(new_size);
    }

    if (new_size == 0) {
        // behaves like free
        kfree(ptr);
        return NULL;
    }

    new_size = (uint32_t)align_up(new_size, 8);

    heap_block_t* block =
    (heap_block_t*)((uintptr_t)ptr - sizeof(heap_block_t));

    uint32_t old_size = block->size;

    // wenn der Block groß genug ist, evtl. nur splitten
    if (new_size <= old_size) {
        split_block(block, new_size);
        return ptr;
    }

    // sonst neuen Block allokieren, kopieren, alten freigeben
    void* new_ptr = kmalloc(new_size);
    if (!new_ptr) {
        return NULL; // kein Speicher
    }

    uint32_t copy_size = (old_size < new_size) ? old_size : new_size;
    kmemcpy(new_ptr, ptr, copy_size);

    kfree(ptr);
    return new_ptr;
}
