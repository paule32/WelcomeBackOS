#pragma once
#include "klog.hpp"
#include "kerror_ctx.hpp"

namespace kx {

inline void log_error_ctx(const ErrorCtx& e, const char* prefix = "ERR") {
    // Basis-Error ausgeben
    log_error(e.base, prefix);

    // Kontext (in Reihenfolge wie gesammelt)
    for (uint32_t i = 0; i < e.ctx_len; ++i) {
        const char* c = e.ctx[i] ? e.ctx[i] : "<?>ctx";
        gfx_printf("  ctx[%u]: %s\n", (unsigned)i, c);
    }
}

__attribute__((noreturn)) inline void panic_error_ctx(const ErrorCtx& e, const char* why = nullptr) {
    if (why) gfx_printf("PANIC: %s\n", why);
    log_error_ctx(e, "PANIC_ERR");
    __builtin_trap();
}

} // namespace kx
