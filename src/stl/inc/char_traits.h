// ----------------------------------------------------------------------------
// \file  char_traits.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __STL_CHAR_TRAITS_H__
#define __STL_CHAR_TRAITS_H__

# include "stdint.h"

namespace std {
    // -------------------------------
    // Minimal char traits (nur was wir brauchen)
    // -------------------------------
    struct char_traits {
        static size_t length(const char* s) {
            if (!s) return 0;
            const char* p = s;
            while (*p) ++p;
            return (size_t)(p - s);
        }
        static int compare(const char* a, const char* b, size_t n) {
            for (size_t i = 0; i < n; ++i) {
                unsigned char ac = (unsigned char)a[i];
                unsigned char bc = (unsigned char)b[i];
                if (ac < bc) return -1;
                if (ac > bc) return  1;
                if (ac == 0) return 0;
            }
            return 0;
        }
        static void copy(char* dst, const char* src, size_t n) {
            for (size_t i = 0; i < n; ++i) dst[i] = src[i];
        }
        static void move(char* dst, const char* src, size_t n) {
            if (dst == src || n == 0) return;
            if (dst < src) {
                for (size_t i = 0; i < n; ++i) dst[i] = src[i];
            } else {
                for (size_t i = n; i-- > 0;) dst[i] = src[i];
            }
        }
    };
}   // namespace: std

#endif  // __STL_CHAR_TRAITS_H__
