// ----------------------------------------------------------------------------
// \file  memory.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __MEMORY_H__
#define __MEMORY_H__

# include "stdint.h"

#ifdef __cplusplus
extern "C" { 
#endif  // __cplusplus

void*    memcpy  (void*,const void*,uint32_t);
void*    memset  (void*,int        ,uint32_t);
int      memcmp  (const void*      ,const void*,size_t);

uint32_t strlen  (const char*);
char*    strcat  (      char*,const char*);
int      strcmp  (const char*,const char*);
int      strncmp (const char*,const char*,uint32_t);

void*    malloc  (uint32_t);
void     free    (void* );

void*    calloc  (size_t,uint32_t);
void*    realloc (void* ,uint32_t);

#ifdef __cplusplus
}; 
#endif  // __cplusplus
#endif  // __MEMORY_H__ 
