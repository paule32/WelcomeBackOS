// ----------------------------------------------------------------------------
// \file  TMenuBar.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __TURBOVISION_TMENUBAR_H__
#define __TURBOVISION_TMENUBAR_H__

# include "stdint.h"

# include "TurboVision/inc/TObject.h"

namespace tvision
{
    class TMenuBar: public TObject {
    public:
        TMenuBar();
        
        void draw(void);
    };
}
#endif  // __TURBOVISION_TMENUBAR_H__
