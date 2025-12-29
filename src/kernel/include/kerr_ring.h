#pragma once
#include <stdint.h>
#include "kerror_ctx.hpp"

namespace kx {

static constexpr uint32_t KX_ERR_RING = 32;

struct ErrRecord {
    ErrorCtx e;
    uint32_t seq;
};

struct ErrRing {
    ErrRecord ring[KX_ERR_RING];
    uint32_t  head; // next write index
    uint32_t  seq;
};

inline ErrRing& err_ring() {
    static ErrRing g{};
    return g;
}

// Nicht thread-safe. Für SMP später mit Spinlock schützen.
inline void err_ring_push(const ErrorCtx& e) {
    ErrRing& r = err_ring();
    const uint32_t idx = r.head % KX_ERR_RING;
    r.ring[idx].e = e;
    r.ring[idx].seq = ++r.seq;
    r.head = (r.head + 1) % KX_ERR_RING;
}

} // namespace kx
