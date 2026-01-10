// paging.c
// (c) 2025 by Jens Kallup - paule32

# include "stdint.h"
# include "proto.h"
# include "paging.h"

# define DESKTOP
# include "vga.h"

# define MMIO_BASE   0xF0000000
static uint32_t next_mmio = MMIO_BASE;

// Symbole aus kernel.ld
extern uint32_t __end;
extern void* kmalloc(uint32_t);

// 4 KiB aligned
static uint32_t page_directory[PAGE_ENTRIES]                __attribute__((aligned(4096)));
static uint32_t page_tables   [PAGE_ENTRIES][PAGE_ENTRIES]  __attribute__((aligned(4096)));
static uint32_t page_table_next_free;

static uint32_t* first_page_table;

# define MANAGED_MEMORY_BYTES (16 * 1024 * 1024)   // 16 MiB
# define MAX_PAGES            (MANAGED_MEMORY_BYTES / PAGE_SIZE)
# define BITMAP_SIZE_BYTES    (MAX_PAGES / 8)

static uint32_t free_pages[MAX_PAGES];
static size_t   free_page_count = 0;

static uint8_t page_bitmap[BITMAP_SIZE_BYTES] __attribute__((aligned(16)));

// Bitmap helpers
static inline void set_bit(uint32_t idx) {
    page_bitmap[idx >> 3] |=  (1u << (idx & 7));
}
static inline void clear_bit(uint32_t idx) {
    page_bitmap[idx >> 3] &= ~(1u << (idx & 7));
}
static inline int test_bit(uint32_t idx) {
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

static inline void invlpg(void *addr) {
    __asm__ volatile("invlpg (%0)" :: "r"(addr) : "memory");
}
static uint32_t* alloc_page_table(void) {
    uint32_t t = page_table_next_free++;
    // Tabelle leeren
    for (uint32_t i = 0; i < 1024; ++i) page_tables[t][i] = 0;
    return page_tables[t];
}
static void map_page(uint32_t virt, uint32_t phys, uint32_t flags)
{
    virt &= PAGE_MASK;
    phys &= PAGE_MASK;

    uint32_t pdi = PD_INDEX(virt);
    uint32_t pti = PT_INDEX(virt);

    if (!(page_directory[pdi] & PG_PRESENT)) {
        uint32_t *pt = alloc_page_table();
        page_directory[pdi] = ((uint32_t)pt & PAGE_MASK) | PG_PRESENT | PG_RW;
    }

    uint32_t *pt = (uint32_t *)(page_directory[pdi] & PAGE_MASK);
    pt[pti] = phys | (flags & 0xFFF) | PG_PRESENT;

    invlpg((void*)virt);
}
void map_range_identity(
    uint32_t phys_start,
    uint32_t size_bytes,
    uint32_t flags) {
        
    uint32_t start = phys_start & PAGE_MASK;
    uint32_t end   = (phys_start + size_bytes + PAGE_SIZE - 1) & PAGE_MASK;

    for (uint32_t p = start; p < end; p += PAGE_SIZE) {
        map_page(p, p, flags);
    }
}

void page_allocator_add_page(uint32_t phys_addr)
{
    // Sicherheit: 4 KiB ausrichten
    phys_addr &= ~(PAGE_SIZE - 1);

    // Optional: ungültige oder zu kleine Bereiche ignorieren
    if (phys_addr < 0x100000) return;  // z.B. alles unter 1 MiB sperren

    if (free_page_count < MAX_PAGES) {
        free_pages[free_page_count++] = phys_addr;
    } else {
        printformat("FC: %d, MAX: %d\n",free_page_count, MAX_PAGES);
        // Optional: Fehlerbehandlung (kein Platz mehr im freien Stack)
    }
}

// reserved_up_to: alle Frames unterhalb dieser Adresse gelten als belegt
void page_init(uint32_t reserved)
{
    uint32_t start = (reserved + 0xFFF) & ~0xFFF;
    uint32_t limit = MANAGED_MEMORY_BYTES;   // 16 MiB, passend zu MAX_PAGES

    for (uint32_t p = start; p < limit; p += 0x1000) {
        page_allocator_add_page(p);
    }
}

void* page_alloc(void)
{
    if (free_page_count == 0) return NULL;
    return (void*)free_pages[--free_page_count];
}

void page_free(void* addr)
{
    if (!addr) return;
    uint32_t a = ((uint32_t)addr) & PAGE_MASK;

    if (a < 0x00100000) return;          // nie unter 1MiB
    if (a >= MANAGED_MEMORY_BYTES) return; // wenn du weiterhin nur 16MiB managst

    if (free_page_count < MAX_PAGES)
        free_pages[free_page_count++] = a;
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
    for (uint32_t i = 0; i < BITMAP_SIZE_BYTES; ++i)
        page_bitmap[i] = 0;

    // 0) first_page_table muss auf echte Page-Table zeigen
    first_page_table = page_tables[0];

    // 1) Page Directory erstmal leer (not present, aber RW gesetzt ist ok)
    for (uint32_t i = 0; i < PAGE_ENTRIES; ++i) {
        page_directory[i] = 0x00000002; // RW, not present
    }

    // 2) Identity-Mapping für NUM_MAPPED_MB (z.B. 16 MiB)
    //    Jede Page-Table mappt 4 MiB => Anzahl Tabellen:
    const uint32_t tables_needed = (NUM_MAPPED_MB + 3) / 4; // aufrunden

    uint32_t phys = 0x00000000;

    for (uint32_t t = 0; t < tables_needed; ++t) {
        // Page Table t füllen
        for (uint32_t i = 0; i < PAGE_ENTRIES; ++i) {
            page_tables[t][i] = (phys & PAGE_MASK) | PAGE_PRESENT | PAGE_WRITE;
            phys += PAGE_SIZE;
        }

        // PDE[t] zeigt auf die Page Table t
        page_directory[t] = ((uint32_t)page_tables[t] & PAGE_MASK) | PAGE_PRESENT | PAGE_WRITE;
    }

    // 3) CR3 laden
    asm volatile("mov %0, %%cr3" :: "r"(page_directory));

    // 4) Paging aktivieren (CR0.PG)
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));

    // kleiner Flush
    asm volatile("jmp .+2");
    
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    vga[0] = 0x0F00 | 'L';
}
