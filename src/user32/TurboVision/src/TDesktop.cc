// ----------------------------------------------------------------------------
// \file  TDesktop.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"

# include "TurboVision/inc/TObject.h"
# include "TurboVision/inc/TDesktop.h"
# include "TurboVision/inc/TMenuBar.h"
# include "TurboVision/inc/TStatusBar.h"

namespace tvision
{
    TDesktop::TDesktop(void)
    {
        auto * menuBar   = new TMenuBar();
        auto * statusBar = new TStatusBar();
    }
    
    TDesktop::~TDesktop()
    {
        delete menuBar;
        delete statusBar;;
    }
    
    void TDesktop::draw(void)
    {
        menuBar  ->draw();
        statusBar->draw();
    }
}   // namespace: tvision
