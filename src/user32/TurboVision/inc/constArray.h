// ----------------------------------------------------------------------------
// \file  constArray.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __TVISION_CONSTARR_H__
#define __TVISION_CONSTARR_H__

# include "stdint.h"

namespace tvision
{
    template <class T, size_t N>
    struct constarray
    {
        T elems[N];

        constexpr T& operator [] (size_t i) {
            return elems[i];
        }

        constexpr const T& operator [] (size_t i) const {
            return elems[i];
        }
    };
}   // namespace tvision

#endif  // __TVISION_CONSTARR_H__
