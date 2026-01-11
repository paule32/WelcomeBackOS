// ----------------------------------------------------------------------------
// \file  ksymbol_table.c
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"

# include "ksymbol_table.h"
# include "proto.h"
# include "kheap.h"

# define DESKTOP
# include "vga.h"

extern "C" void clear_screen(unsigned char, unsigned char);
extern "C" void settextcolor(unsigned char, unsigned char);

static kernel_symbol_t kernel_symbols[] = {
    { (uintptr_t)&kmalloc      , KSIG_KMALLOC__UINT32            },
    { (uintptr_t)&kfree        , KSIG_KFREE__VOID_PTR            },
    
    { (uintptr_t)&printformat  , KSIG_PRINTFORMAT__CCHAR_PTR_ANY },
    
    { (uintptr_t) static_cast<clear_screen__uc_uc>( &clear_screen ), KSIG_CLEARSCREEN__VOID  },
    { (uintptr_t) static_cast<setTextColor__uc_uc>( &settextcolor ), KSIG_SETTEXTCOLOR_UC_UC },
    
    // ---------------------------------------------------------
    // graphics: gfx_drawCicle
    // ---------------------------------------------------------
        
    // ---------------------------------------------------------
    // graphics: gfx_drawLine
    // ---------------------------------------------------------

    // ---------------------------------------------------------
    // graphics: gfx_rectFill
    // ---------------------------------------------------------
    { (uintptr_t) static_cast<rectfill__u16ptriiiiitc >( &gfx_rectFill ), KSIG_GFX_RECTFiLL__UINT16_PTR_INT_INT_INT_INT_INT_TCOLOR },
    { (uintptr_t) static_cast<rectfill__iiii_us       >( &gfx_rectFill ), KSIG_GFX_RECTFiLL__INT_INT_INT_INT_USHORT       },
    { (uintptr_t) static_cast<rectfill__iiii_tc       >( &gfx_rectFill ), KSIG_GFX_RECTFiLL__INT_INT_INT_INT_TCOLOR       },
    { (uintptr_t) static_cast<rectfill__ii_tc         >( &gfx_rectFill ), KSIG_GFX_RECTFiLL__INT_INT_TCOLOR               },
    { (uintptr_t) static_cast<rectfill__pt_tc         >( &gfx_rectFill ), KSIG_GFX_RECTFiLL__TPOINT_REF_INT_PTR_TCOLOR    },
    { (uintptr_t) static_cast<rectfill__pt_iptr_tc    >( &gfx_rectFill ), KSIG_GFX_RECTFiLL__TPOINT_REF_TCOLOR            },
    { (uintptr_t) static_cast<rectfill__tr_tc         >( &gfx_rectFill ), KSIG_GFX_RECTFiLL__TRECT_REF_TCOLOR             },
};

static const uint32_t kernel_symbols_count =
    (uint32_t)(sizeof(kernel_symbols) /
               sizeof(kernel_symbols[0]));

kernel_symbol_t* get_kernel_symbol_list(void) {
    return kernel_symbols;
}

uint32_t get_kernel_symbol_count(void) {
    return kernel_symbols_count;
}
