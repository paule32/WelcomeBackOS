// ----------------------------------------------------------------------------
// \file  strings.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
//
// \note  Turbo Vision - Version 2.0
//        Copyright (c) 1994 by Borland International
//        All Rights Reserved.
// ----------------------------------------------------------------------------
# include "tv.h"
# include "stdint.h"
# include "TurboVision/inc/constArray.h"
# include "TurboVision/inc/strings.h"

namespace tvision
{
    static constexpr
    size_t _fast_utoa(uint32_t value, char *buffer)
    {
        // Copyright(c) 2014-2016 Milo Yip (https://github.com/miloyip/itoa-benchmark)
        size_t digits =
              value < 10          ? 1
            : value < 100         ? 2
            : value < 1000        ? 3
            : value < 10000       ? 4
            : value < 100000UL    ? 5
            : value < 1000000UL   ? 6
            : value < 10000000UL  ? 7
            : value < 100000000UL ? 8
            : value < 1000000000UL? 9 : 10;
        buffer += digits;
        do {
            *--buffer = char(value % 10) + '0';
            value /= 10;
        } while (value > 0);

        return digits;
    }

    char* fast_utoa(uint32_t value, char *buffer) {
        return buffer + _fast_utoa(value, buffer);
    }

    # define BTOA_CONSTEXPR     constexpr
    # define BTOA_CONSTEXPR_VAR const

    static BTOA_CONSTEXPR
    btoa_lut_t init_btoa_lut()
    {
        btoa_lut_t res {};
        for (uint32_t i = 0; i < 256; ++i)
            res[i].digits = _fast_utoa(i, res[i].chars);
        return res;
    }

extern BTOA_CONSTEXPR_VAR
btoa_lut_t btoa_lut = init_btoa_lut();

}   // namespace: tvision
