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
# include "TurboVision/inc/TEvent.h"

namespace tvision
{
    class TEvent;
    class TView: public TObject
    {
    public:
        TRect rect;
        
        // Geometrie später (TRect). Fürs Event-System reicht:
        bool  selectable;
        bool  focused;
        TView *owner;         // Parent (Group)

        void draw() virtual;
        bool handleEvent(TEvent *ev);
    };
}   // namespace: tvision
#endif  // __TVISION_TVIEW_H__
