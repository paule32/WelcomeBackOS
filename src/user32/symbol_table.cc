// ----------------------------------------------------------------------------
// \file  symbol_table.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# define IMPORT

# include "stdint.h"
# include "stdarg.h"
# include "memory.h"

# include "iso9660.h"
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

__imp__kmemsetw__ushort_ptr_ushort_int_t __imp__kmemsetw__ushort_ptr_ushort_int = nullptr;
__imp__kmemcpy__void_ptr_cvoid_ptr_int_t __imp__kmemcpy__void_ptr_cvoid_ptr_int = nullptr;

__imp__mmio_map__int_int_t __imp__mmio_map__int_int = nullptr;

// ----------------------------------------------------------------------------
// text screen Variablen (Funktionszeiger)
// ----------------------------------------------------------------------------
__imp__printformat__cchar_ptr_any_t   __imp__printformat__cchar_ptr_any = nullptr;

__imp__clear_screen__uc_uc_t          __imp__clear_screen__uc_uc = nullptr;
__imp__setTextColor__uc_uc_t          __imp__setTextColor__uc_uc = nullptr;

// ----------------------------------------------------------------------------
// string convertion function types
// ----------------------------------------------------------------------------
__imp__kitoa__int_char_t        __imp__kitoa__int_char    = nullptr;
__imp__ki2hex__uint_char_int_t  __imp__ki2hex__uint_char_int = nullptr;

// ----------------------------------------------------------------------------
// file operation function's typs ...
// ----------------------------------------------------------------------------
__imp__file_open__cchar_t                 __imp__file_open__cchar                = nullptr;
__imp__file_read__fileptr_void_ptr_int_t  __imp__file_read__fileptr_void_ptr_int = nullptr;
__imp__file_close__fileptr_t              __imp__file_close__fileptr             = nullptr;

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
    IMPORT_SYM( KSIG_KMEMSETW__USHORT_PTR_USHORT_INT, kmemsetw__ushort_ptr_ushort_int);
    IMPORT_SYM( KSIG_KMEMCPY__VOID_PTR_CVOID_PTR_INT, kmemcpy__void_ptr_cvoid_ptr_int);
    
    IMPORT_SYM( KSIG_MMIO_MAP__INT_INT, mmio_map__int_int);
    IMPORT_SYM( KSIG_PRINTFORMAT__CCHAR_PTR_ANY, printformat__cchar_ptr_any );
    
    IMPORT_SYM( KSIG_CLEARSCREEN__VOID , clear_screen__uc_uc );
    IMPORT_SYM( KSIG_SETTEXTCOLOR_UC_UC, setTextColor__uc_uc );

    // ---------------------------------------------------------
    // string convertion functions ...
    // ---------------------------------------------------------
    IMPORT_SYM( KSIG_KITOA__INT_CHAR, kitoa__int_char);
    IMPORT_SYM( KSIG_KI2HEX__UINT_CHAR_INT, ki2hex__uint_char_int);
    
    // ---------------------------------------------------------
    // file operation function's ...
    // ---------------------------------------------------------
    IMPORT_SYM( KSIG_FILEOPEN__CCHAR   , file_open__cchar );
    IMPORT_SYM( KSIG_FILECLOSE__FILEPTR, file_close__fileptr );
    IMPORT_SYM( KSIG_FILEREAD__FILEPTR_VOID_PTR_INT, file_read__fileptr_void_ptr_int);
    
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
void* malloc(uint32_t size) {
    if (__imp__kmalloc__uint32 != nullptr) {    // todo !
       return (__imp__kmalloc__uint32)(size);
    }  return nullptr;
}

void free(void* ptr) {
    if (__imp__kfree__void_ptr != nullptr) {
       (__imp__kfree__void_ptr)(ptr);
    }
}

extern "C" USHORT* kmemsetw(USHORT* dest, USHORT val, size_t count) {
    if (__imp__kmemsetw__ushort_ptr_ushort_int != nullptr) {
        return (__imp__kmemsetw__ushort_ptr_ushort_int)(dest, val, count);
    }   return nullptr;
}

void* mmio_map(uint32_t phys, uint32_t size) {
    if (__imp__mmio_map__int_int != nullptr) {
        return (__imp__mmio_map__int_int)(phys, size);
    }   return nullptr;
}

extern "C" void  kfree(void* ptr) { free(ptr); }
extern "C" void* kmalloc(uint32_t size) { return malloc(size); }

extern "C" void* kmemcpy(void* dst, const void* src, uint32_t n) {
    if (__imp__kmemcpy__void_ptr_cvoid_ptr_int != nullptr) {
        return (__imp__kmemcpy__void_ptr_cvoid_ptr_int)(dst, src, n);
    }   return nullptr;
}

void clear_screen(unsigned char fg, unsigned char bg) {
    if (__imp__clear_screen__uc_uc != nullptr) {
       (__imp__clear_screen__uc_uc)(fg, bg);
    }
}

void setTextColor(uint8_t fg, uint8_t bg)
{
    if (__imp__setTextColor__uc_uc != nullptr) {
       (__imp__setTextColor__uc_uc)(fg, bg);
    }
}

void printformat(const char* fmt, ...)
{
    if (__imp__printformat__cchar_ptr_any != nullptr) {
        va_list ap;
        va_start(ap, fmt);
        (__imp__printformat__cchar_ptr_any)(fmt, ap);
        va_end(ap);
    }
}

FILE* file_open(const char *path) {
    if (__imp__file_open__cchar != nullptr) {
        return (__imp__file_open__cchar)(path);
    }   return nullptr;
}
void file_close(FILE* fd)
{
    if (__imp__file_close__fileptr != nullptr) {
       (__imp__file_close__fileptr)(fd);
    }
}

extern "C" uint32_t file_read(FILE* fd, void* buf, uint32_t len)
{
    if (__imp__file_read__fileptr_void_ptr_int != nullptr) {
        return (__imp__file_read__fileptr_void_ptr_int)(fd, buf, len);
    }   return 0;
}


extern "C" void kitoa(int value, char* valuestring) {
    if (__imp__kitoa__int_char != nullptr) {
       (__imp__kitoa__int_char)(value, valuestring);
    }
}

extern "C" void ki2hex(UINT val, char* dest, int len) {
    if (__imp__ki2hex__uint_char_int != nullptr) {
       (__imp__ki2hex__uint_char_int)(val, dest, len);
    }
}