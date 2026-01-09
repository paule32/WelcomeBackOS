// ----------------------------------------------------------------------------
// \file  ksymbol_table.c
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"

# include "ksymbol_table.h"
# include "proto.h"
# include "kheap.h"

extern "C" void clear_screen(void);

static kernel_symbol_t kernel_symbols[] = {
    { "malloc",        (uintptr_t)&kmalloc      , KSIG_KMALLOC     },
    { "free",          (uintptr_t)&kfree        , KSIG_KFREE       },
    { "printformat",   (uintptr_t)&printformat  , KSIG_PRINTFORMAT },
    { "ConsoleClear",  (uintptr_t)&clear_screen , KSIG_CLEARSCREEN },
};

static const uint32_t kernel_symbols_count =
    (uint32_t)(sizeof(kernel_symbols) /
               sizeof(kernel_symbols[0]));

extern "C" kernel_symbol_t* get_kernel_symbol_list(void) {
    return kernel_symbols;
}

extern "C" uint32_t get_kernel_symbol_count(void) {
    return kernel_symbols_count;
}
