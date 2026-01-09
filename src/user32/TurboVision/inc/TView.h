// ----------------------------------------------------------------------------
// \file  TView.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
//
// \note  Turbo Vision - Version 2.0
//        Copyright (c) 1994 by Borland International
//        All Rights Reserved.
// ----------------------------------------------------------------------------
#ifndef __TVISION_TVIEW_H__
#define __TVISION_TVIEW_H__

# include "stdint.h"
# include "TurboVision/inc/TObject.h"
# include "TurboVision/inc/TRect.h"

namespace tvision {
    class TView: public TObject {
    public:
        enum phaseType  { phFocused, phPreProcess, phPostProcess };
        enum selectMode { normalSelect, enterSelect, leaveSelect };

         TView(const TRect& bounds);
        ~TView();
    };
}   // namespace: std
#endif  // __TVISION_TVIEW_H__
