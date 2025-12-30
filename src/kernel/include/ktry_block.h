// ----------------------------------------------------------------------------
// \file  ktry_block.h
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __KTRY_BLOCK_H__
#define __KTRY_BLOCK_H__

# include "kerror.h"   // kx::Result / kx::Error
# include "klog.h"     // kx::log_error (optional)
# include "ktry.h"     // für KX_HERE / KX_ERR

// --------------------------------------------------------
// Nutzung:
//
// KX_TRY_BLOCK(mein_code())
//     // CATCH-Teil: _kx_err ist verfügbar (kx::Error)
// KX_OK
//     // OK-Teil
// KX_END_TRY
//
// Optional: KX_TRY_BLOCK_LOG(...) loggt automatisch.
//
// --------------------------------------------------------
# define KX_TRY_BLOCK(expr)                               \
    if (auto _kx_try_res = (expr); !_kx_try_res) {        \
        auto _kx_err = _kx_try_res.error();

# define KX_TRY_BLOCK_LOG(expr)                           \
    if (auto _kx_try_res = (expr); !_kx_try_res) {        \
        auto _kx_err = _kx_try_res.error();               \
        ::kx::log_error(_kx_err, "CATCH");

// ------------------
// value vom Result:
# define KX_TRY_BLOCK_VAL(name, expr)                        \
    if (auto _kx_try_res_##name = (expr); !_kx_try_res_##name) { \
        auto _kx_err = _kx_try_res_##name.error();

# define KX_TRY_BLOCK_VAL_LOG(name, expr)                        \
    if (auto _kx_try_res_##name = (expr); !_kx_try_res_##name) { \
        auto _kx_err = _kx_try_res_##name.error();               \
        ::kx::log_error(_kx_err, "CATCH");                       \
    } else {                                                     \
        auto name = _kx_try_res_##name.value();

# define KX_OK_VAL(name) } else { auto name = _kx_try_res_##name.value();

# define KX_OK } else {

# define KX_END_TRY }

#define KX_LOG_CATCH() \
    ::kx::log_error(_kx_err, "CATCH")

// --------------------------------------------------------
// KX_TRY_BLOCK_VAL(n, parse_u32("123"))
//    KX_IF_ERR(kx::Err::NoMem) {
//        gfx_printf("OOM while parsing\n");
//    } KX_ELSE_ERR {
//        gfx_printf("parse failed: %u\n", (unsigned)_kx_err.code);
//    }
// KX_OK_VAL(n)
//    gfx_printf("n=%d\n", n);
// KX_END_TRY
// --------------------------------------------------------
# define KX_IF_ERROR(cod) if (_kx_err.code == (cod)) {
# define KX_ELSE_ERR else
# define KX_ELSE } else {
# define KX_ENDIF }

/*
// Beispiel: parse_u32 kann „Speicherfehler“ zurückgeben:
kx::Result<int> parse_u32(const char* s) {
    // ... irgendwas braucht Speicher ...
    void* p = kmalloc(16);
    if (!p) return KX_ERR(kx::Err::NoMem, "Speicherfehler");

    // ... parse ...
    kfree(p);
    return 123;
}

// dann: ...
//
KX_TRY_BLOCK_VAL(n, parse_u32("123"))
    // CATCH: wird ausgeführt, wenn parse_u32() Err::NoMem (oder irgendwas anderes) zurückgibt
    if (_kx_err.code == kx::Err::NoMem) {
        gfx_printf("OOM!\n");
    } else {
        gfx_printf("anderer Fehler\n");
    }
KX_OK_VAL(n)
    // OK: wird ausgeführt, wenn parse_u32 erfolgreich ist
    gfx_printf("n=%d\n", n);
KX_END_TRY

-----------------

// OK-Wert + Error automatisch loggen (ein Makro)
// Wenn du im Fehlerfall immer sofort etwas sehen willst:

KX_TRY_BLOCK_VAL_LOG(n, parse_u32("123"))
    // OK-Block (n ist verfügbar)
    gfx_printf("n=%d\n", n);
KX_END_TRY

------------------

KX_TRY_BLOCK_VAL(n, parse_u32("123"))
    KX_LOG_CATCH();
KX_OK_VAL(n)
    gfx_printf("n=%d\n", n);
KX_END_TRY

-----------------
*/

#endif  // __KTRY_BLOCK_H__
