#ifndef PAGING_H
#define PAGING_H

# include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

# define PAGE_SIZE       4096
# define PAGE_ENTRIES    1024

# define PAGE_MASK       0xFFFFF000

# define PD_INDEX(v)     (((v) >> 22) & 0x3FF)
# define PT_INDEX(v)     (((v) >> 12) & 0x3FF)

// Page-Flags
# define PG_PRESENT      0x001
# define PG_RW           0x002
# define PG_USER         0x004  // für Kernel eher 0
# define PG_PWT          0x008
# define PG_PCD          0x010
# define PG_GLOBAL       0x100

# define NUM_MAPPED_MB   16
# define NUM_PAGES       ((NUM_MAPPED_MB * 1024 * 1024) / PAGE_SIZE)
# define NUM_TABLES      ((NUM_PAGES + PAGE_ENTRIES - 1) / PAGE_ENTRIES)

# define PAGE_PRESENT    0x001
# define PAGE_WRITE      0x002
# define PAGE_USER       0x004

# define PD_INDEX(v)    (((v) >> 22) & 0x3FF)
# define PT_INDEX(v)    (((v) >> 12) & 0x3FF)

extern uint32_t __end;

// reserved_up_to: alle phys. Adressen < reserved_up_to gelten als belegt (Kernel+Heap)
// wir verwalten nur 0..16 MiB (kann später erweitert werden)
extern void  paging_init(void);

extern void  page_init(uint32_t);
extern void* page_alloc(void );  // liefert 4-KiB-seitiges physisches Frame (identity-mapped)
extern void  page_free (void*);  // gibt eine Seite wieder frei

extern void map_range_identity(uint32_t,uint32_t,uint32_t);

#ifdef __cplusplus
};
#endif  // __cplusplus

#endif
