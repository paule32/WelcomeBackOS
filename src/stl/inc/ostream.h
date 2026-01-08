// ----------------------------------------------------------------------------
// \file  ostream.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __STL_OSTREAM_H__
#define __STL_OSTREAM_H__

// freestanding minimal types
# include "stdint.h"
# include "kheap.h"

namespace std { 
    class ostream {
    public:
        // Du setzt diesen Hook einmal beim Init.
        using write_fn = void(*)(const char*, size_t);
        write_fn write = nullptr;

        ostream& put(char c) {
            if (write) write(&c, 1);
            return *this;
        }

        ostream& write_str(const char* s) {
            if (!s) s = "(null)";
            size_t n = char_traits::length(s);
            if (write && n) write(s, n);
            return *this;
        }

        // --- operator<< Überladungen
        ostream& operator<<(const char* s) { return write_str(s); }
        ostream& operator<<(char c)        { return put(c); }
        ostream& operator<<(const string& s){ return write_str(s.c_str()); }

        ostream& operator<<(endl_t) {
            put('\n');
            return *this;
        }

        // integer printing (minimal)
        ostream& operator<<(sint16_t v)  { return print_signed((sint16_t)v); }
        ostream& operator<<(sint32_t v)  { return print_signed((sint32_t)v); }
        
        ostream& operator<<(uint16_t v)  { return print_unsigned((uint16_t)v); }
        ostream& operator<<(uint32_t v)  { return print_unsigned((uint32_t)v); }

        ostream& hex(uint32_t v) { // helper: "0x..."
            write_str("0x");
            return print_hex(v);
        }

    private:
        ostream& print_signed(sint32_t v) {
            if (v < 0) { put('-'); return print_unsigned((uint32_t)(-v)); }
            return print_unsigned((uint32_t)v);
        }
        ostream& print_unsigned(uint32_t v) {
            char buf[32];
            size_t i = 0;
            if (v == 0) { put('0'); return *this; }
            while (v && i < sizeof(buf)) {
                buf[i++] = char('0' + (v % 10));
                v /= 10;
            }
            while (i) put(buf[--i]);
            return *this;
        }
        ostream& print_hex(uint32_t v) {
            char buf[32];
            const char* digits = "0123456789ABCDEF";
            size_t i = 0;
            if (v == 0) { put('0'); return *this; }
            while (v && i < sizeof(buf)) {
                buf[i++] = digits[(size_t)(v & 0xF)];
                v >>= 4;
            }
            while (i) put(buf[--i]);
            return *this;
        }
    };
}   // namespace std
 
#endif  // __STL_OSTREAM_H__
