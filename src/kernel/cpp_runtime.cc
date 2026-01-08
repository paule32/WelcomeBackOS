// ----------------------------------------------------------------------------
// \file  cpp_runtime.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"
# include "kheap.h"

extern "C" void* kmalloc(size_t);
extern "C" void  kfree(void*);

static __attribute__((noreturn)) void kernel_oom() {
    __builtin_trap();
}

// new/delete
void* operator new(size_t n) {
    if (n == 0) n = 1;
    void* p = kmalloc(n);
    if (!p) kernel_oom();
    return p;
}
void operator delete(void* p) noexcept {
    if (p) kfree(p);
}

// new[]/delete[]
void* operator new[](size_t n) { return operator new(n); }
void operator delete[](void* p) noexcept { operator delete(p); }

// sized delete (Toolchain-Kompatibilität)
void operator delete(void* p, size_t) noexcept { operator delete(p); }
void operator delete[](void* p, size_t) noexcept { operator delete[](p); }

// pure virtual
extern "C" void __cxa_pure_virtual() { __builtin_trap(); }

// atexit (weil wir keine Destruktoren beim Shutdown fahren)
extern "C" int __cxa_atexit(void (*)(void*), void*, void*) { return 0; }
extern "C" void* __dso_handle() { return (void*)0; }
