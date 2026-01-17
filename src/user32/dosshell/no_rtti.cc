// ----------------------------------------------------------------------------
// \file  no_rtti.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#include "stdint.h"

extern "C" void* malloc(uint32_t);
extern "C" void  free(void*);

void* operator new  (uint32_t n) { return malloc((uint32_t)n); }
void* operator new[](uint32_t n) { return malloc((uint32_t)n); }

void  operator delete  (void* p, unsigned int) { if (p) free(p); }
void  operator delete[](void* p, unsigned int) { if (p) free(p); }

void  operator delete  (void* p) { if (p) free(p); }
void  operator delete[](void* p) { if (p) free(p); }
