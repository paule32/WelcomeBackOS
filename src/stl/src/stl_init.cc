// ----------------------------------------------------------------------------
// \file  stl_init.cc
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"
# include "iostream.h"

namespace std {

void* kmalloc(uint32_t s) {
    return (void*)s;
}
void kfree(void* p) {
    p = nullptr;
}
// Diese beiden Symbole musst du im Kernel an deine kmalloc/kfree binden.
// Wenn du sie nicht hast: mach simple wrapper um deinen Heap.
extern "C" void* mini_malloc(uint32_t n) {
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
