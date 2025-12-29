#pragma once
#include "kerror_ctx.hpp"
#include "klog_ctx.hpp"
#include "ktry.hpp" // für KX_HERE / KX_ERR

// Wir nutzen weiterhin KX_ERR(...) für Source-Location.
// Jetzt kommt Kontext obendrauf.

#define KX_CTX(expr, ctxmsg)                                          \
    do {                                                              \
        auto _kx_r = (expr);                                          \
        if (!_kx_r) {                                                 \
            auto _e = ::kx::with_ctx(_kx_r.error(), (ctxmsg));         \
            ::kx::log_error_ctx(_e, "PROP");                          \
            return _e;                                                \
        }                                                             \
    } while (0)

// TRY mit Kontext und outVar
#define KX_TRY_CTX(outVar, expr, ctxmsg)                              \
    auto _kx_tmp_##outVar = (expr);                                   \
    if (!_kx_tmp_##outVar) {                                          \
        auto _e = ::kx::with_ctx(_kx_tmp_##outVar.error(), (ctxmsg));  \
        ::kx::log_error_ctx(_e, "PROP");                              \
        return _e;                                                    \
    }                                                                 \
    auto outVar = _kx_tmp_##outVar.value()

// ENSURE -> ErrorCtx (und loggt)
#define KX_ENSURE_CTX(cond, code, msg, ctxmsg)                        \
    do {                                                              \
        if (!(cond)) {                                                \
            auto _e0 = ::kx::ctx_from(KX_ERR((code), (msg)));          \
            auto _e  = ::kx::with_ctx(_e0, (ctxmsg));                 \
            ::kx::log_error_ctx(_e, "ENSURE");                        \
            return _e;                                                \
        }                                                             \
    } while (0)

#define KX_TRY_OR_PANIC(outVar, expr)                             \
    auto _kx_tmp_##outVar = (expr);                               \
    if (!_kx_tmp_##outVar) {                                      \
        ::kx::panic_error(_kx_tmp_##outVar.error(), "TRY_OR_PANIC");\
    }                                                             \
    auto outVar = _kx_tmp_##outVar.value()
