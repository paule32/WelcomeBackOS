// kheap.c
// Very simple first-fit kernel heap
// (c) 2025 by Jens Kallup - paule32

# include "stdint.h"
# include "proto.h"
# include "kheap.h"

// Symbol from linker script: first free byte after kernel + bss
extern "C" uint32_t __end; 

// 1 MiB heap size (can be adjusted)
# define KHEAP_SIZE   (0x00100000)   // 1 MiB

typedef struct heap_block {
    uint32_t size;              // size of the usable area (in bytes)
    uint8_t  free;              // 1 = free, 0 = used
    uint8_t  _pad[3];           // padding for alignment
    struct heap_block* next;    // next block in list
}   heap_block_t;

alignas(16) static uint8_t kheap_area[KHEAP_SIZE];
static heap_block_t* heap_head = nullptr;

//static heap_block_t* heap_head = (heap_block_t*)NULL;

         mem_map_entry_t* mem_map;
uint32_t mem_map_length; // Anzahl Einträge
uint32_t max_mem = 0;    // oberstes Ende des nutzbaren physikalischen RAMs

// simple helper: align up to 'align' (must be power of 2)
static inline uint32_t align_up(uint32_t value, uint32_t align)
{
    return (value + align - 1) & ~(align - 1);
}

void detect_memory(void)
{
    uint32_t highest = 0;

    for (uint32_t i = 0; i < mem_map_length; ++i) {
        mem_map_entry_t* e = &mem_map[i];
        if (e->type != 1) continue;

        uint32_t end = (uint32_t)e->base + (uint32_t)e->length;
        if (end > highest) highest = end;
    }

    if (highest > 0xFFFFFFFF) highest = 0xFFFFFFFF;
    max_mem = (uint32_t)highest;
}

// simple memcpy, weil keine libc vorhanden ist
extern "C" void* kmemcpy(void* dst, const void* src, uint32_t n)
{
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;

    for (uint32_t i = 0; i < n; ++i)
    d[i] = s[i];

    return dst;
}

extern "C" void* kmemset(void* dst, int value, uint32_t n)
{
    uint8_t* d = (uint8_t*)dst;

    for (uint32_t i = 0; i < n; ++i)
    d[i] = (uint8_t)value;

    return dst;
}

USHORT* kmemsetw(USHORT* dest, USHORT val, size_t count)
{
    USHORT* temp = (USHORT*) dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

extern "C" size_t kstrlen(const char* str)
{
    size_t retval;

    for(retval = 0; *str != '\0'; ++str)
    ++retval;

    return retval;
}

extern "C" int kstrncmp(const char* a, const char* b, uint32_t n)
{
    for (uint32_t i = 0; i < n; ++i) {
        if (a[i] != b[i]) return (int)((unsigned char)a[i] - (unsigned char)b[i]);
        if (a[i] == 0) return 0;
    }
    return 0;
}

extern "C" int kstrcmp(const char* a, const char* b)
{
    while (*a && *b && *a == *b) {
        ++a; ++b;
    }
    return (int)((unsigned char)*a - (unsigned char)*b);
}

extern "C" char* kstrcat(char* dest, const char* src)
{
    char *d = dest;

    // ans Ende von dest
    while (*d != '\0') {
        d++;
    }

    // src Zeichen für Zeichen kopieren
    const char *s = src;
    while (*s != '\0') {
        *d = *s;
        d++;
        s++;
    }

    // Nullterminator setzen
    *d = '\0';

    return dest;
}

void kheap_init(void)
{
    // Start of heap directly after the kernel
    uint32_t start = (uint32_t)(uintptr_t)kheap_area;
    start = align_up(start, 8);
    //(uint32_t)&__end;

    // Align heap start to 8 bytes for sanity
    //start = align_up((uint32_t)start, 8);

    heap_head = (heap_block_t*)start;

    // total heap space = KHEAP_SIZE minus header
    heap_head->size = KHEAP_SIZE - sizeof(heap_block_t);
    heap_head->free = 1;
    heap_head->next = nullptr;
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

extern "C" void* kmalloc(uint32_t size)
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

extern "C" void kfree(void* ptr)
{
    if (!ptr) return;

    // get header
    heap_block_t* block =
    (heap_block_t*)((uintptr_t)ptr - sizeof(heap_block_t));

    block->free = 1;

    // try to merge neighboring free blocks
    merge_free_blocks();
}

extern "C" void* krealloc(void* ptr, uint32_t new_size)
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

void* malloc(size_t n) { return kmalloc((uint32_t)n); }
void  free(void* p)    { if (p) kfree(p); }
void* calloc(size_t c, size_t s) {
    size_t n = c*s;
    uint8_t* p = (uint8_t*)kmalloc((uint32_t)n);
    if (!p) return NULL;
    for (size_t i=0;i<n;i++) p[i]=0;
    return p;
}
void* realloc(void* p, size_t n) {
return krealloc(p, (uint32_t)n);
}
