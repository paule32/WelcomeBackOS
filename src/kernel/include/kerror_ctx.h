#pragma once
#include <stdint.h>
#include "kerror.hpp"

namespace kx {

// Max. Anzahl Kontext-Einträge pro Fehler (klein halten -> Kernel)
static constexpr uint32_t KX_MAX_CTX = 6;

struct ErrorCtx {
    Error base;
    const char* ctx[KX_MAX_CTX];   // z.B. "loading iso9660", "opening /foo"
    uint32_t ctx_len;
};

inline ErrorCtx ctx_from(Error e) {
    ErrorCtx r{};
    r.base = e;
    r.ctx_len = 0;
    for (uint32_t i = 0; i < KX_MAX_CTX; ++i) r.ctx[i] = nullptr;
    return r;
}

inline ErrorCtx with_ctx(ErrorCtx e, const char* c) {
    if (c && e.ctx_len < KX_MAX_CTX) {
        e.ctx[e.ctx_len++] = c;
    }
    return e;
}

inline ErrorCtx with_ctx(Error e, const char* c) {
    return with_ctx(ctx_from(e), c);
}

// Result, das ErrorCtx statt Error trägt
template <typename T>
class ResultCtx {
    union { T value_; };
    ErrorCtx err_;
    bool ok_;

public:
    ResultCtx(const T& v) : value_(v), err_(ctx_from(Error::ok())), ok_(true) {}
    ResultCtx(T&& v)      : value_((T&&)v), err_(ctx_from(Error::ok())), ok_(true) {}

    ResultCtx(ErrorCtx e) : err_(e), ok_(false) {}
    ResultCtx(Error e)    : err_(ctx_from(e)), ok_(false) {}

    ~ResultCtx() { if (ok_) value_.~T(); }

    ResultCtx(const ResultCtx& o) : err_(o.err_), ok_(o.ok_) {
        if (ok_) new (&value_) T(o.value_);
    }
    ResultCtx& operator=(const ResultCtx& o) {
        if (this == &o) return *this;
        if (ok_) value_.~T();
        err_ = o.err_;
        ok_  = o.ok_;
        if (ok_) new (&value_) T(o.value_);
        return *this;
    }

    bool ok() const { return ok_; }
    explicit operator bool() const { return ok_; }

    T&       value()       { return value_; }
    const T& value() const { return value_; }

    ErrorCtx error() const { return err_; }
};

template <>
class ResultCtx<void> {
    ErrorCtx err_;
    bool ok_;
public:
    ResultCtx() : err_(ctx_from(Error::ok())), ok_(true) {}
    ResultCtx(ErrorCtx e) : err_(e), ok_(false) {}
    ResultCtx(Error e)    : err_(ctx_from(e)), ok_(false) {}

    bool ok() const { return ok_; }
    explicit operator bool() const { return ok_; }
    ErrorCtx error() const { return err_; }
};

} // namespace kx
