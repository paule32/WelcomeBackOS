#pragma once
#include <stddef.h>

extern "C" void kfree(void*);

namespace kx {

template <typename T>
struct KFreeDeleter {
    void operator()(T* p) const { if (p) kfree((void*)p); }
};

template <typename T, typename D = KFreeDeleter<T>>
class UniquePtr {
    T* p_;
    D  d_;

public:
    UniquePtr() : p_(nullptr), d_() {}
    explicit UniquePtr(T* p) : p_(p), d_() {}
    ~UniquePtr() { reset(); }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& o) : p_(o.p_), d_(o.d_) { o.p_ = nullptr; }
    UniquePtr& operator=(UniquePtr&& o) {
        if (this != &o) { reset(); p_ = o.p_; o.p_ = nullptr; }
        return *this;
    }

    T* get() const { return p_; }
    T* release() { T* t = p_; p_ = nullptr; return t; }
    void reset(T* np = nullptr) { if (p_) d_(p_); p_ = np; }

    explicit operator bool() const { return p_ != nullptr; }
    T& operator*() const { return *p_; }
    T* operator->() const { return p_; }
};

} // namespace kx
