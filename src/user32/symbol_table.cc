// ----------------------------------------------------------------------------
// \file  symbol_table.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# define IMPORT

# include "stdint.h"
# include "memory.h"

# include "ksymbol_table.h"

kernel_symbol_t *kernel_symbols       = nullptr;
uint32_t         kernel_symbols_count = 0;

// ----------------------------------------------------------------------------
// Lookup nach Name
// ----------------------------------------------------------------------------
#if 0
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
#endif

// ----------------------------------------------------------------------------
// Lookup nach Signatur
// ----------------------------------------------------------------------------
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
__imp__kmalloc__uint32_t  __imp__kmalloc__uint32 = nullptr;
__imp__kfree__void_ptr_t  __imp__kfree__void_ptr = nullptr;

// ----------------------------------------------------------------------------
// text screen Variablen (Funktionszeiger)
// ----------------------------------------------------------------------------
__imp__printformat__cchar_ptr_any_t   __imp__printformat__cchar_ptr_any = nullptr;
__imp__clear_screen__void_t           __imp__clear_screen__void = nullptr;
__imp__clear_screen2__void_t          __imp__clear_screen2__void = nullptr;

// ----------------------------------------------------------------------------
// graphics variables ...
// ----------------------------------------------------------------------------
__imp__gfx_drawCircle__int_int_int_int_ushort_t              __imp__gfx_drawCircle__int_int_int_int_ushort = nullptr;
__imp__gfx_drawCircle__int_int_int_ushort_t                  __imp__gfx_drawCircle__int_int_int_ushort = nullptr;
__imp__gfx_drawCircleFill__int_int_int_ushort_t              __imp__gfx_drawCircleFill__int_int_int_ushort = nullptr;

__imp__gfx_drawLine__int_int_int_int_int_ushort_t            __imp__gfx_drawLine__int_int_int_int_int_ushort = nullptr;
__imp__gfx_drawLine__uint16_ptr_int_int_int_int_int_ushort_t __imp__gfx_drawLine__uint16_ptr_int_int_int_int_int_ushort = nullptr;

__imp__gfx_rectFill__uint16_ptr_int_int_int_int_int_TColor_t __imp__gfx_rectFill__uint16_ptr_int_int_int_int_int_TColor = nullptr;
__imp__gfx_rectFill__int_int_int_int_ushort_t                __imp__gfx_rectFill__int_int_int_int_ushort = nullptr;
__imp__gfx_rectFill__int_int_int_int_TColor_t                __imp__gfx_rectFill__int_int_int_int_TColor = nullptr;
__imp__gfx_rectFill__int_int_TColor_t                        __imp__gfx_rectFill__int_int_TColor = nullptr;
__imp__gfx_rectFill__TPoint_ref_TColor_t                     __imp__gfx_rectFill__TPoint_ref_TColor = nullptr;
__imp__gfx_rectFill__TPoint_ref_int_ptr_TColor_t             __imp__gfx_rectFill__TPoint_ref_int_ptr_TColor = nullptr;
__imp__gfx_rectFill__TRect_ref_TColor_t                      __imp__gfx_rectFill__TRect_ref_TColor = nullptr;

// ----------------------------------------------------------------------------
// initialize the kernel symbol table (import functions) ...
// ----------------------------------------------------------------------------
void kernel_symbols_init(void)
{
    const kernel_symbol_t* sym;
    
    IMPORT_SYM( KSIG_KMALLOC__UINT32, kmalloc__uint32);
    IMPORT_SYM( KSIG_KFREE__VOID_PTR, kfree__void_ptr);
    
    IMPORT_SYM( KSIG_PRINTFORMAT__CCHAR_PTR_ANY, printformat__cchar_ptr_any );
    IMPORT_SYM( KSIG_CLEARSCREEN__VOID, clear_screen__void );
    IMPORT_SYM( KSIG_CLEARSCREEN2__VOID, clear_screen2__void );

    // ---------------------------------------------------------
    // graphics: gfx_drawCicle
    // ---------------------------------------------------------
    IMPORT_SYM( KSIG_GFX_DRAWCIRCLE__INT_INT_INT_INT_USHORT , gfx_drawCircle__int_int_int_int_ushort );
    IMPORT_SYM( KSIG_GFX_DRAWCIRCLE__INT_INT_INT_USHORT     , gfx_drawCircle__int_int_int_ushort     );
    IMPORT_SYM( KSIG_GFX_DRAWCIRCLEFILL__INT_INT_INT_USHORT , gfx_drawCircleFill__int_int_int_ushort );
    
    // ---------------------------------------------------------
    // graphics: gfx_drawLine
    // ---------------------------------------------------------
    IMPORT_SYM( KSIG_GFX_DRAWLINE__INT_INT_INT_INT_INT_USHORT           , gfx_drawLine__int_int_int_int_int_ushort            );
    IMPORT_SYM( KSIG_GFX_DRAWLINE__UINT16_PTR_INT_INT_INT_INT_INT_USHORT, gfx_drawLine__uint16_ptr_int_int_int_int_int_ushort );
    
    // ---------------------------------------------------------
    // graphics: gfx_rectFill
    // ---------------------------------------------------------
    IMPORT_SYM( KSIG_GFX_RECTFiLL__UINT16_PTR_INT_INT_INT_INT_INT_TCOLOR, gfx_rectFill__uint16_ptr_int_int_int_int_int_TColor );
    IMPORT_SYM( KSIG_GFX_RECTFiLL__INT_INT_INT_INT_USHORT,                gfx_rectFill__int_int_int_int_ushort                );
    IMPORT_SYM( KSIG_GFX_RECTFiLL__INT_INT_INT_INT_TCOLOR,                gfx_rectFill__int_int_int_int_TColor                );
    IMPORT_SYM( KSIG_GFX_RECTFiLL__INT_INT_TCOLOR,                        gfx_rectFill__int_int_TColor                        );
    IMPORT_SYM( KSIG_GFX_RECTFiLL__TPOINT_REF_INT_PTR_TCOLOR,             gfx_rectFill__TPoint_ref_int_ptr_TColor             );
    IMPORT_SYM( KSIG_GFX_RECTFiLL__TPOINT_REF_TCOLOR,                     gfx_rectFill__TPoint_ref_TColor                     );
    IMPORT_SYM( KSIG_GFX_RECTFiLL__TRECT_REF_TCOLOR,                      gfx_rectFill__TRect_ref_TColor                      );
}

// ----------------------------------------------------------------------------
// standard functions for common usage:
// ----------------------------------------------------------------------------
void* malloc(uint32_t count) {
    if (__imp__kmalloc__uint32 != nullptr) {    // todo !
       return (__imp__kmalloc__uint32)(count);
    }  return nullptr;
}

void free(void* ptr) {
    if (__imp__kfree__void_ptr != nullptr) {
       (__imp__kfree__void_ptr)(ptr);
    }
}

void clear_screen(void) {
    if (__imp__clear_screen__void != nullptr) {
       (__imp__clear_screen__void)();
    }
}

void clear_screen2(void) {
    if (__imp__clear_screen2__void != nullptr) {
       (__imp__clear_screen2__void)();
    }
}
