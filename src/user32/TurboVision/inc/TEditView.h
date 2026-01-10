// ----------------------------------------------------------------------------
// \file  TDesktop.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __TURBOVISION_TEDITVIEW_H__
#define __TURBOVISION_TEDITVIEW_H__

# include "stdint.h"
# include "TurboVision/inc/TView.h"

typedef struct TView  TView;
struct TEditView {
    TView base;
    char  buf[256];
    uint32_t len;
    uint32_t cursor;
    const char *title;
};

#endif  // __TURBOVISION_TEDITVIEW_H__
