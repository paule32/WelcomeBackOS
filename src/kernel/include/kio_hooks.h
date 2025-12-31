// ----------------------------------------------------------------------------
// \file  kio_hooks.h
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "kstl.h"

static void kernel_write_impl(const char* s, size_t n) {
    // z.B. VGA/Framebuffer/Serial-Ausgabe
    extern void gfx_write_bytes(const char* s, size_t n);
    gfx_write_bytes(s, n);
}

static size_t kernel_read_impl(char* dst, size_t n) {
    // z.B. Tastaturpuffer
    // Rückgabe: wie viele Bytes wirklich geliefert wurden (0 wenn nichts)
    extern size_t kbd_read(char* dst, size_t n);
    return kbd_read(dst, n);
}

void mini_stl_init() {
    std::cout.write = &kernel_write_impl;
    std::cin.read   = &kernel_read_impl;
}
