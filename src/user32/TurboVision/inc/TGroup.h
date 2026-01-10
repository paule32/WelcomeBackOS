// ----------------------------------------------------------------------------
// \file  TGroup.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __TURBOVISION_TGROUP_H__
#define __TURBOVISION_TGROUP_H__

# include "stdint.h"
# include "TurboVision/inc/TView.h"

struct TGroup {
    TView base;
    TView *children[8];
    uint32_t count;
    TView *current;
};

#endif  // __TURBOVISION_TGROUP_H__
