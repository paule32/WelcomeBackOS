// ----------------------------------------------------------------------------
// \file  ksymbol_table.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __KSYMBOL_TABLE_H__
#define __KSYMBOL_TABLE_H__

# include "stdint.h"

# define DESKTOP
# include "vga.h"

typedef enum {
    // ---------------------------------------------------------
    // memory
    // ---------------------------------------------------------
    KSIG_KMALLOC__UINT32,
    KSIG_KFREE__VOID_PTR,
    
    // ---------------------------------------------------------
    // text console
    // ---------------------------------------------------------
    KSIG_PRINTFORMAT__CCHAR_PTR_ANY,

    KSIG_CLEARSCREEN__VOID,
    KSIG_SETTEXTCOLOR_UC_UC,
    
    // ---------------------------------------------------------
    // graphics: gfx_drawCicle
    // ---------------------------------------------------------
    KSIG_GFX_DRAWCIRCLE__INT_INT_INT_INT_USHORT,
    KSIG_GFX_DRAWCIRCLE__INT_INT_INT_USHORT,
    KSIG_GFX_DRAWCIRCLEFILL__INT_INT_INT_USHORT,
    
    // ---------------------------------------------------------
    // graphics: gfx_drawLine
    // ---------------------------------------------------------
    KSIG_GFX_DRAWLINE__INT_INT_INT_INT_INT_USHORT,
    KSIG_GFX_DRAWLINE__UINT16_PTR_INT_INT_INT_INT_INT_USHORT,
    
    // ---------------------------------------------------------
    // graphics: gfx_rectFill
    // ---------------------------------------------------------
    KSIG_GFX_RECTFiLL__UINT16_PTR_INT_INT_INT_INT_INT_TCOLOR,
    KSIG_GFX_RECTFiLL__INT_INT_INT_INT_USHORT,
    KSIG_GFX_RECTFiLL__INT_INT_INT_INT_TCOLOR,
    KSIG_GFX_RECTFiLL__INT_INT_TCOLOR,
    KSIG_GFX_RECTFiLL__TPOINT_REF_INT_PTR_TCOLOR,
    KSIG_GFX_RECTFiLL__TPOINT_REF_TCOLOR,
    KSIG_GFX_RECTFiLL__TRECT_REF_TCOLOR,
    
}   kernel_sig_t;

typedef struct {
    uintptr_t   func_off;   // offset address
    uint16_t    func_sig;   // signature as id
}   kernel_symbol_t;

extern kernel_symbol_t* get_kernel_symbol_list (void);
extern uint32_t         get_kernel_symbol_count(void);

// ----------------------------------------------------------------------------
// symbol export castings ...
// ----------------------------------------------------------------------------
using rectfill__u16ptriiiiitc  = void(*)(uint16_t*, int,int,int,int,int, TColor);
using rectfill__iiii_us        = void(*)(int,int,int,int, USHORT);
using rectfill__iiii_tc        = void(*)(int,int,int,int, TColor);
using rectfill__ii_tc          = void(*)(int,int, TColor);
using rectfill__pt_tc          = void(*)(TPoint&, TColor);
using rectfill__pt_iptr_tc     = void(*)(TPoint&, int*, TColor);
using rectfill__tr_tc          = void(*)(TRect&, TColor);

using clear_screen__uc_uc      = void(*)(unsigned char, unsigned char);
using setTextColor__uc_uc      = void(*)(unsigned char, unsigned char);

#ifdef IMPORT

extern kernel_symbol_t *kernel_symbols;
extern uint32_t         kernel_symbols_count;

// ----------------------------------------------------------------------------
// Typen ...
// ----------------------------------------------------------------------------
typedef void* (*__imp__kmalloc__uint32_t            )(uint32_t);
typedef void  (*__imp__kfree__void_ptr_t            )(void*);

typedef int   (*__imp__printformat__cchar_ptr_any_t )(const char* fmt, ...);
typedef void  (*__imp__clear_screen__uc_uc_t        )(unsigned char, unsigned char);
typedef void  (*__imp__setTextColor__uc_uc_t        )(unsigned char, unsigned char);
// ----------------------------------------------------------------------------
// graphics types ...
// ----------------------------------------------------------------------------
typedef void  (*__imp__gfx_drawCircle__int_int_int_int_ushort_t )(int,int,int,int,USHORT);
typedef void  (*__imp__gfx_drawCircle__int_int_int_ushort_t     )(int,int,int,    USHORT);
typedef void  (*__imp__gfx_drawCircleFill__int_int_int_ushort_t )(int,int,int,    USHORT);

typedef void  (*__imp__gfx_drawLine__int_int_int_int_int_ushort_t            )(          int,int,int,int,int, USHORT);
typedef void  (*__imp__gfx_drawLine__uint16_ptr_int_int_int_int_int_ushort_t )(uint16_t*,int,int,int,int,int, USHORT);

typedef void  (*__imp__gfx_rectFill__uint16_ptr_int_int_int_int_int_TColor_t )(uint16_t*,int,int,int,int,int, TColor);
typedef void  (*__imp__gfx_rectFill__int_int_int_int_ushort_t                )(int,int,int,int,               USHORT);
typedef void  (*__imp__gfx_rectFill__int_int_int_int_TColor_t                )(int,int,int,int,               TColor);
typedef void  (*__imp__gfx_rectFill__int_int_TColor_t                        )(int,int,                       TColor);
typedef void  (*__imp__gfx_rectFill__TPoint_ref_TColor_t                     )(TPoint&,                       TColor);
typedef void  (*__imp__gfx_rectFill__TPoint_ref_int_ptr_TColor_t             )(TPoint&,int*,                  TColor);
typedef void  (*__imp__gfx_rectFill__TRect_ref_TColor_t                      )(TRect&,                        TColor);

// ----------------------------------------------------------------------------
// memory Variablen (Funktionszeiger)
// ----------------------------------------------------------------------------
extern __imp__kmalloc__uint32_t        __imp__kmalloc__uint32;
extern __imp__kfree__void_ptr_t        __imp__kfree__void_ptr;

extern __imp__printformat__cchar_ptr_any_t     __imp__printformat__cchar_ptr_any;

// ----------------------------------------------------------------------------
// text screen Variablen (Funktionszeiger)
// ----------------------------------------------------------------------------
extern __imp__clear_screen__uc_uc_t   __imp__clear_screen__uc_uc;
extern __imp__setTextColor__uc_uc_t   __imp__setTextColor__uc_uc;

// ----------------------------------------------------------------------------
// graphics variables ...
// ----------------------------------------------------------------------------
extern __imp__gfx_drawCircle__int_int_int_int_ushort_t              __imp__gfx_drawCircle__int_int_int_int_ushort;
extern __imp__gfx_drawCircle__int_int_int_ushort_t                  __imp__gfx_drawCircle__int_int_int_ushort;
extern __imp__gfx_drawCircleFill__int_int_int_ushort_t              __imp__gfx_drawCircleFill__int_int_int_ushort;

extern __imp__gfx_drawLine__int_int_int_int_int_ushort_t            __imp__gfx_drawLine__int_int_int_int_int_ushort;
extern __imp__gfx_drawLine__uint16_ptr_int_int_int_int_int_ushort_t __imp__gfx_drawLine__uint16_ptr_int_int_int_int_int_ushort;

extern __imp__gfx_rectFill__uint16_ptr_int_int_int_int_int_TColor_t __imp__gfx_rectFill__uint16_ptr_int_int_int_int_int_TColor;
extern __imp__gfx_rectFill__int_int_int_int_ushort_t                __imp__gfx_rectFill__int_int_int_int_ushort;
extern __imp__gfx_rectFill__int_int_int_int_TColor_t                __imp__gfx_rectFill__int_int_int_int_TColor;
extern __imp__gfx_rectFill__int_int_TColor_t                        __imp__gfx_rectFill__int_int_TColor;
extern __imp__gfx_rectFill__TPoint_ref_TColor_t                     __imp__gfx_rectFill__TPoint_ref_TColor;
extern __imp__gfx_rectFill__TPoint_ref_int_ptr_TColor_t             __imp__gfx_rectFill__TPoint_ref_int_ptr_TColor;
extern __imp__gfx_rectFill__TRect_ref_TColor_t                      __imp__gfx_rectFill__TRect_ref_TColor;

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
