#ifndef PAGING_H
#define PAGING_H

# include "stdint.h"

# define PAGE_SIZE 4096

extern uint8_t _end;

// reserved_up_to: alle phys. Adressen < reserved_up_to gelten als belegt (Kernel+Heap)
// wir verwalten nur 0..16 MiB (kann später erweitert werden)
extern void  paging_init(void);

extern void  page_init(uint32_t);
extern void* page_alloc(void );  // liefert 4-KiB-seitiges physisches Frame (identity-mapped)
extern void  page_free (void*);  // gibt eine Seite wieder frei

#endif
