// paging.c
// (c) 2025 by Jens Kallup - paule32

# include "stdint.h"
# include "proto.h"
# include "vga.h"

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
# define PAGE_WRITE      0x002
# define PAGE_USER       0x004

# define MMIO_BASE   0xF0000000
static uint32_t next_mmio = MMIO_BASE;

// Symbole aus kernel.ld
extern uint32_t __end;
extern void* kmalloc(uint32_t);

// 4 KiB aligned
static uint32_t page_directory[PAGE_ENTRIES]                __attribute__((aligned(4096)));
static uint32_t page_tables   [PAGE_ENTRIES][PAGE_ENTRIES]  __attribute__((aligned(4096)));
static uint32_t* first_page_table;

static uint32_t free_pages[PAGE_ENTRIES];
static size_t   free_page_count = 0;

# define MANAGED_MEMORY_BYTES (16 * 1024 * 1024)   // 16 MiB
# define MAX_PAGES            (MANAGED_MEMORY_BYTES / PAGE_SIZE)
# define BITMAP_SIZE_BYTES    (MAX_PAGES / 8)

static uint8_t page_bitmap[BITMAP_SIZE_BYTES];

// Bitmap helpers
static inline void set_bit(uint32_t idx) {
    page_directory[idx >> 3] |=  (1u << (idx & 7));
}

static inline void clear_bit(uint32_t idx) {
    page_directory[idx >> 3] &= ~(1u << (idx & 7));
}

static inline uint8_t test_bit(uint32_t idx) {
    return (page_directory[idx >> 3] >> (idx & 7)) & 1u;
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

void page_allocator_add_page(uint32_t phys_addr)
{
    // Sicherheit: 4 KiB ausrichten
    phys_addr &= ~(PAGE_SIZE - 1);

    // Optional: ungültige oder zu kleine Bereiche ignorieren
    // if (phys_addr < 0x100000) return;  // z.B. alles unter 1 MiB sperren

    if (free_page_count < MAX_PAGES) {
        free_pages[free_page_count++] = phys_addr;
    } else {
        // Optional: Fehlerbehandlung (kein Platz mehr im freien Stack)
    }
}

// reserved_up_to: alle Frames unterhalb dieser Adresse gelten als belegt
void page_init(uint32_t reserved)
{
    // alles < reserved ist belegt
    // zusätzlich: framebuffer-Bereich blocken
    vbe_info_t* vi   = (vbe_info_t*)0x800; 
    uint32_t fb_phys = vi->phys_base;
    uint32_t fb_size = vi->xres * vi->yres * (vi->bpp / 8);

    uint32_t fb_start =  fb_phys & ~0xFFF;
    uint32_t fb_end   = (fb_phys + fb_size + 0xFFF) & ~0xFFF;

    for (uint32_t p = reserved; p < max_mem; p += 0x1000) {
        if (p >= fb_start && p < fb_end) {
            continue; // VRAM nicht in freien Pool aufnehmen
        }
        page_allocator_add_page(p);
    }
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

    // Erste Page Table füllen: identity mapping 0x00000000–0x003FFFFF
    uint32_t phys = 0x00000000;
    for (int i = 0; i < PAGE_ENTRIES; ++i) {
        first_page_table[i] = (phys & 0xFFFFF000) | PAGE_PRESENT | PAGE_WRITE;
        phys += PAGE_SIZE;   // nächste physische Seite
    }
    
    // PDE[0] zeigt auf first_page_table
    uint32_t pt_phys = (uint32_t)first_page_table;
    page_directory[0] = (pt_phys & 0xFFFFF000) | PAGE_PRESENT | PAGE_WRITE;
    
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
