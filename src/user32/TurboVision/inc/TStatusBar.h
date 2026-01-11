// ----------------------------------------------------------------------------
// \file  TStatusBar.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __TURBOVISION_TSTATUSBAR_H__
#define __TURBOVISION_TSTATUSBAR_H__

# include "stdint.h"

# include "TurboVision/inc/TObject.h"

namespace tvision
{
    class TStatusBar: public TObject {
    public:
        TStatusBar(void);
        
        void draw(void);
    };
}
#endif  // __TURBOVISION_TSTATUSBAR_H__
