// ----------------------------------------------------------------------------
// \file  ksymbol_table.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __KSYMBOL_TABLE_H__
#define __KSYMBOL_TABLE_H__

# include "stdint.h"

typedef enum {
    KSIG_KMALLOC,
    KSIG_KFREE,
    KSIG_PRINTFORMAT,
    KSIG_CLEARSCREEN
}   kernel_sig_t;

typedef struct {
    const char* func_idn;   // function id-name
    uintptr_t   func_off;   // offset address
    uint16_t    func_sig;   // signature as id
}   kernel_symbol_t;

extern kernel_symbol_t* get_kernel_symbol_list (void);
extern uint32_t         get_kernel_symbol_count(void);

#ifdef IMPORT

extern kernel_symbol_t *kernel_symbols;
extern uint32_t         kernel_symbols_count;

// ----------------------------------------------------------------------------
// Typen
// ----------------------------------------------------------------------------
typedef void* (*__imp__kmalloc_t      )(uint32_t);
typedef void  (*__imp__kfree_t        )(void*);
typedef int   (*__imp__printformat_t  )(const char* fmt, ...);
typedef void  (*__imp__clear_screen_t )(void);

// ----------------------------------------------------------------------------
// Variablen (Funktionszeiger)
// ----------------------------------------------------------------------------
extern __imp__kmalloc_t         __imp__kmalloc;
extern __imp__kfree_t           __imp__kfree;
extern __imp__printformat_t     __imp__printformat;
extern __imp__clear_screen_t    __imp__clear_screen;

// ----------------------------------------------------------------------------
// Helper Makro, um Code-Platz zu sparen ...
// ----------------------------------------------------------------------------
# define IMPORT_SYM(SIG, NAME)                                     \
    do {                                                           \
        sym = ksym_find_sig((SIG));                                \
        if (sym) {                                                 \
            __imp__##NAME = (__imp__##NAME##_t)(sym->func_off);    \
        }                                                          \
    } while (0)

#endif

#endif  // __KSYMBOL_TABLE_H__
