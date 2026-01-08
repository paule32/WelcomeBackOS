// ----------------------------------------------------------------------------
// \file  kstl.cc
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "kstl.h"

namespace std {

// Diese beiden Symbole musst du im Kernel an deine kmalloc/kfree binden.
// Wenn du sie nicht hast: mach simple wrapper um deinen Heap.
extern "C" void* mini_malloc(size_t n) {
    extern void* kmalloc(size_t);
    return kmalloc(n);
}

extern "C" void mini_free(void* p) {
    extern void kfree(void*);
    kfree(p);
}

// globale Instanzen
ostream cout{};
istream cin{};

} // namespace std
