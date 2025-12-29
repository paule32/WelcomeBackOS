#pragma once
#include "ktry.hpp"

#define KX_PANIC_ON_ERROR(expr)                                   \
    do {                                                          \
        auto _kx_tmp = (expr);                                    \
        if (!_kx_tmp) {                                           \
            ::kx::panic_error(_kx_tmp.error(), "PANIC_ON_ERROR");  \
        }                                                         \
    } while (0)

#define KX_PANIC_ON_NULL(ptr, msg)                                \
    do {                                                          \
        if ((ptr) == nullptr) {                                   \
            ::kx::panic(msg);                                     \
        }                                                         \
    } while (0)
