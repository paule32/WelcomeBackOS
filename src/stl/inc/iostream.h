// ----------------------------------------------------------------------------
// \file  iostream.h
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __STL_IOSTREAM_H__
#define __STL_IOSTREAM_H__

// freestanding minimal types
# include "stdint.h"
# include "kheap.h"

# include "stl/inc/istream.h"
# include "stl/inc/ostream.h"

template <class T>
constexpr const T& min(const T& a, const T& b) {
    return (b < a) ? b : a;
}

namespace std {
    // globale Streams
    extern ostream cout;
    extern istream cin;

}   // namespace std

#endif  // __STL_IOSTREAM_H__
