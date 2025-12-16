// paging.c
// (c) 2025 by Jens Kallup - paule32

# include "stdint.h"

# define PAGE_SIZE       4096
# define PAGE_ENTRIES    1024

# define PAGE_MASK       0xFFFFF000

# define PD_INDEX(v)     (((v) >> 22) & 0x3FF)
# define PT_INDEX(v)     (((v) >> 12) & 0x3FF)

// Page-Flags
# define PG_PRESENT      0x001
# define PG_RW           0x002
# define PG_USER         0x004  // für Kernel eher 0
# define PG_GLOBAL       0x100

# define NUM_MAPPED_MB   16
# define NUM_PAGES       ((NUM_MAPPED_MB * 1024 * 1024) / PAGE_SIZE)
# define NUM_TABLES      ((NUM_PAGES + PAGE_ENTRIES - 1) / PAGE_ENTRIES)

# define PAGE_PRESENT    0x001
# define PAGE_RW         0x002

# define MMIO_BASE   0xF0000000
static uint32_t next_mmio = MMIO_BASE;

// Symbole aus kernel.ld
extern uint32_t __end;

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

// Hilfsfunktionen, um CR3 zu lesen/schreiben
static inline uint32_t read_cr3(void) {
    uint32_t val;
    __asm__ __volatile__("mov %%cr3, %0" : "=r"(val));
    return val;
}
static inline void write_cr3(uint32_t val) {
    __asm__ __volatile__("mov %0, %%cr3" :: "r"(val) : "memory");
}

// Identity-Mapping für niedrigen Speicher angenommen:
// phys == virt für Bereiche, in denen das Page Directory liegt.
static inline uint32_t *get_page_directory(void) {
    return (uint32_t *)page_directory;
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

void *mmio_map(uint32_t phys, uint32_t size)
{
    uint32_t phys_aligned = phys &  PAGE_MASK;
    uint32_t offset       = phys & ~PAGE_MASK;

    uint32_t size_total   = size + offset;
    uint32_t num_pages    = (size_total + PAGE_SIZE - 1) / PAGE_SIZE;

    uint32_t vaddr_start  = next_mmio;
    uint32_t vaddr        = vaddr_start;
    uint32_t paddr        = phys_aligned;

    uint32_t *pd = get_page_directory();

    for (uint32_t i = 0; i < num_pages; ++i) {
        uint32_t pd_idx = PD_INDEX(vaddr);
        uint32_t pt_idx = PT_INDEX(vaddr);

        // PDE prüfen
        if (!(pd[pd_idx] & PG_PRESENT)) {
            // neue Page Table holen
            uint32_t pt_phys = (uint32_t)page_alloc();  // physische 4KB-Page
            // Page Table phys == virt angenommen (Identity-Mapping unten)
            uint32_t *pt = (uint32_t *)pt_phys;

            // Page Table nullen
            for (int j = 0; j < 1024; ++j) pt[j] = 0;

            pd[pd_idx] = pt_phys | PG_PRESENT | PG_RW;
        }

        // Page Table virt ermitteln
        uint32_t pt_phys = pd[pd_idx] & PAGE_MASK;
        uint32_t *pt = (uint32_t *)pt_phys;   // bei Identity-Mapping

        pt[pt_idx] = (paddr & PAGE_MASK) | PG_PRESENT | PG_RW;

        vaddr += PAGE_SIZE;
        paddr += PAGE_SIZE;
    }

    // TLB flush – CR3 neu laden
    uint32_t cr3 = read_cr3();
    write_cr3(cr3);

    next_mmio = vaddr;

    return (void *)(vaddr_start + offset);
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
