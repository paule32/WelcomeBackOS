#ifndef KHEAP_H
#define KHEAP_H

# include "stdint.h"

extern void  kheap_init(void);
extern void* kmalloc   (uint32_t);
extern void* krealloc  (void*,uint32_t);
extern void  kfree     (void*);
extern void* kmemcpy   (void*,const void*,uint32_t);
extern void* kmemset   (void*,int        ,uint32_t);

#endif
