#ifndef __KHEAP_H__
#define __KHEAP_H__

# include "stdint.h"

#ifdef __cplusplus
extern "C" { 
#endif  // __cplusplus

void  kheap_init(void);
void* kmalloc   (uint32_t);
void* krealloc  (void*,uint32_t);
void  kfree     (void*);
void* kmemcpy   (void*,const void*,uint32_t);
void* kmemset   (void*,int        ,uint32_t);

size_t kstrlen  (const char*);
char*  kstrcat  (      char*,const char*);
int    kstrcmp  (const char*,const char*);
int    kstrncmp (const char*,const char*,uint32_t);

extern USHORT* kmemsetw(USHORT* dest, USHORT val, size_t count);

#ifdef __cplusplus
}; 
#endif  // __cplusplus
#endif  // __KHEAP_H__ 
