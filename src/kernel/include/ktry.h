// ----------------------------------------------------------------------------
// \file  ktry.h
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#pragma once
#include "kerror.h"
#include "klog.h"

namespace kx {

// Factory ohne Makro-Zwang: du gibst SrcLoc explizit (nutzt unten Helper)
inline Error err(Err code, const char* msg, SrcLoc loc) {
    return make_error(code, msg, loc);
}

// Helper-Makro NUR für Source-Location (optional, aber ultra praktisch)
#define KX_HERE ::kx::here(__FILE__, (uint32_t)__LINE__, __func__)

// Noch ein Helper für Fehlererzeugung (optional)
#define KX_ERR(code, msg) ::kx::make_error((code), (msg), KX_HERE)

// TRY: ohne Makro-Zwang als Funktion geht nur eingeschränkt (weil "out var").
// Darum: schlanke Makros, aber mit Auto-Logging:

#define KX_TRY(outVar, expr)                                      \
    auto _kx_tmp_##outVar = (expr);                               \
    if (!_kx_tmp_##outVar) {                                      \
        ::kx::log_error(_kx_tmp_##outVar.error(), "PROP");         \
        return _kx_tmp_##outVar.error();                          \
    }                                                             \
    auto outVar = _kx_tmp_##outVar.value()

#define KX_TRYV(expr)                                             \
    do {                                                          \
        auto _kx_tmp = (expr);                                    \
        if (!_kx_tmp) {                                           \
            ::kx::log_error(_kx_tmp.error(), "PROP");             \
            return _kx_tmp.error();                               \
        }                                                         \
    } while (0)

// ENSURE mit Auto-Logging
#define KX_ENSURE(cond, code, msg)                                \
    do {                                                          \
        if (!(cond)) {                                            \
            auto _e = KX_ERR((code), (msg));                      \
            ::kx::log_error(_e, "ENSURE");                        \
            return _e;                                            \
        }                                                         \
    } while (0)

} // namespace kx
