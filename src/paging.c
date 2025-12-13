// paging.c
// (c) 2025 by Jens Kallup - paule32

# include "stdint.h"

# define PAGE_SIZE       4096
# define PAGE_ENTRIES    1024

# define NUM_MAPPED_MB   16
# define NUM_PAGES       ((NUM_MAPPED_MB * 1024 * 1024) / PAGE_SIZE)
# define NUM_TABLES      ((NUM_PAGES + PAGE_ENTRIES - 1) / PAGE_ENTRIES)

# define PAGE_PRESENT    0x001
# define PAGE_RW         0x002

// Symbole aus kernel.ld
extern uint8_t _end;

// 4 KiB aligned
static uint32_t page_directory[PAGE_ENTRIES]                __attribute__((aligned(4096)));
static uint32_t page_tables   [PAGE_ENTRIES][PAGE_ENTRIES]  __attribute__((aligned(4096)));

# define MANAGED_MEMORY_BYTES (16 * 1024 * 1024)   // 16 MiB
# define MAX_PAGES            (MANAGED_MEMORY_BYTES / PAGE_SIZE)
# define BITMAP_SIZE_BYTES    (MAX_PAGES / 8)

static uint8_t page_bitmap[BITMAP_SIZE_BYTES];

// Bitmap helpers
static inline void set_bit(uint32_t idx) {
    page_bitmap[idx >> 3] |=  (1u << (idx & 7));
}

static inline void clear_bit(uint32_t idx) {
    page_bitmap[idx >> 3] &= ~(1u << (idx & 7));
}

static inline uint8_t test_bit(uint32_t idx) {
    return (page_bitmap[idx >> 3] >> (idx & 7)) & 1u;
}

// reserved_up_to: alle Frames unterhalb dieser Adresse gelten als belegt
void page_init(uint32_t reserved_up_to)
{
    // alles erstmal als frei markieren
    for (uint32_t i = 0; i < BITMAP_SIZE_BYTES; ++i) {
        page_bitmap[i] = 0x00;
    }

    // alle Frames unterhalb 'reserved_up_to' als belegt markieren
    uint32_t reserved_pages =
        (reserved_up_to + PAGE_SIZE - 1) / PAGE_SIZE;  // aufrunden

    if (reserved_pages > MAX_PAGES) {
        reserved_pages = MAX_PAGES;
    }

    for (uint32_t i = 0; i < reserved_pages; ++i) {
        set_bit(i);
    }

    // Frames oberhalb reserved_up_to bleiben frei (bit = 0)
}

void* page_alloc(void)
{
    for (uint32_t i = 0; i < MAX_PAGES; ++i) {
        if (!test_bit(i)) {
            // freie Seite gefunden
            set_bit(i);
            // Identity-Mapping: virtuelle == physische Adresse
            uint32_t addr = i * PAGE_SIZE;
            return (void*)addr;
        }
    }

    return NULL; // kein freier Frame
}

void page_free(void* addr)
{
    if (!addr) return;

    uint32_t a = (uint32_t)addr;
    if (a >= MANAGED_MEMORY_BYTES) {
        // Adresse ausserhalb unseres Verwaltungsbereichs
        return;
    }

    uint32_t page_idx = (uint32_t)(a / PAGE_SIZE);
    clear_bit(page_idx);
}

void paging_init(void)
{
    // 1. Alles erstmal "nicht present"
    for (uint32_t i = 0; i < PAGE_ENTRIES; ++i) {
        page_directory[i] = 0x00000002;   // RW, aber not present
    }

    // 2. Page Tables für 0..16 MiB
    uint32_t phys = 0;
    for (uint32_t t = 0; t < NUM_TABLES; ++t) {
        for (uint32_t i = 0; i < PAGE_ENTRIES; ++i) {
            if (phys < NUM_MAPPED_MB * 1024 * 1024) {
                page_tables[t][i] = phys | PAGE_PRESENT | PAGE_RW;
                phys += PAGE_SIZE;
            } else {
                page_tables[t][i] = 0x00000002; // RW, not present
            }
        }
        page_directory[t] = ((uint32_t)page_tables[t]) | PAGE_PRESENT | PAGE_RW;
    }
    
    // 3. CR3 = physische Adresse des Page Directory
    asm volatile("mov %0, %%cr3" :: "r"(page_directory));

    // 4. Paging-Bit in CR0 setzen
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));

    // kleine Pipeline-Flush
    asm volatile("jmp .+2");
    
    // Ab hier ist Paging aktiv, aber mit Identity-Mapping,
    // d.h. deine Adressen unter 4 MiB bleiben gleich.
}
