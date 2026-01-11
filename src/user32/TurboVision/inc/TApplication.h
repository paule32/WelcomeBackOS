// ----------------------------------------------------------------------------
// \file  TApplication.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __TURBOVISION_TAPPLICATION_H__
#define __TURBOVISION_TAPPLICATION_H__

# include "stdint.h"

# include "TurboVision/inc/TObject.h"
//# include "TurboVision/inc/TGroup.h"
//# include "TurboVision/inc/TEditView.h"

namespace tvision
{
    class TApplication: public TObject
    {
    public:
                 TApplication(void);
        virtual ~TApplication() {}
        
        virtual void init();
        int  run();
    private:
         bool running = false;
    };
}   // namespace: tvision
#endif  // __TURBOVISION_TAPPLICATION_H__
