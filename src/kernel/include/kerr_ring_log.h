#pragma once
#include "kerr_ring.hpp"
#include "klog_ctx.hpp"

namespace kx {

inline void log_and_store(const ErrorCtx& e, const char* prefix = "ERR") {
    err_ring_push(e);
    log_error_ctx(e, prefix);
}

inline void dump_last_errors(uint32_t max = 16) {
    ErrRing& r = err_ring();
    gfx_printf("---- last errors (max %u) ----\n", (unsigned)max);

    // wir laufen rückwärts von head-1
    uint32_t count = 0;
    for (uint32_t i = 0; i < KX_ERR_RING && count < max; ++i) {
        uint32_t pos = (r.head + KX_ERR_RING - 1 - i) % KX_ERR_RING;
        const ErrRecord& rec = r.ring[pos];
        if (rec.seq == 0) break; // uninitialisiert
        gfx_printf("#%u\n", (unsigned)rec.seq);
        log_error_ctx(rec.e, "HIST");
        ++count;
    }
    gfx_printf("-----------------------------\n");
}

} // namespace kx
