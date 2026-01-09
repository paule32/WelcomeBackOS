// ----------------------------------------------------------------------------
// \file  symbol_table.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# define IMPORT

# include "stdint.h"
# include "ksymbol_table.h"

kernel_symbol_t *kernel_symbols       = nullptr;
uint32_t         kernel_symbols_count = 0;

// ----------------------------------------------------------------------------
// Lookup nach Name
// ----------------------------------------------------------------------------
const kernel_symbol_t* ksym_find(const char* name)
{
    for (size_t i = 0; i < kernel_symbols_count; ++i) {
        const char* a = kernel_symbols[i].func_idn;
        const char* b = name;
        
        while (*a && *b && *a == *b) { ++a; ++b; }
        
        if (*a == '\0' && *b == '\0')
        return &kernel_symbols[i];
    }   return nullptr;
}

const kernel_symbol_t* ksym_find_sig(kernel_sig_t sig)
{
    for (size_t i = 0; i < kernel_symbols_count; ++i)
        if (kernel_symbols[i].func_sig == sig)
            return &kernel_symbols[i];
    return nullptr;
}

// ----------------------------------------------------------------------------
// importierte Funktionen ...
// ----------------------------------------------------------------------------
__imp__kmalloc_t        __imp__kmalloc      = 0;
__imp__kfree_t          __imp__kfree        = 0;
__imp__printformat_t    __imp__printformat  = 0;
__imp__clear_screen_t   __imp__clear_screen = 0;

void kernel_symbols_init(void)
{
    const kernel_symbol_t* sym;
    
    IMPORT_SYM( KSIG_KMALLOC, kmalloc);
    IMPORT_SYM( KSIG_KFREE, kfree);
    IMPORT_SYM( KSIG_PRINTFORMAT, printformat);
    IMPORT_SYM( KSIG_CLEARSCREEN, clear_screen);
}

void clear_screen(void)
{
    if (__imp__clear_screen != nullptr) {
       (__imp__clear_screen)();
    }
}
