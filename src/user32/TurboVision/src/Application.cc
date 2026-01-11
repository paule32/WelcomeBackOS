// ----------------------------------------------------------------------------
// \file  Application.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"

#include "TurboVision/inc/TApplication.h"

extern void printformat(const char* args, ...);

namespace tvision
{
    TApplication::TApplication(void)
    {
        auto * desktop = new TDesktop();
     }
    TApplication::~TApplication()
    {
        delete desktop;
    }
    
    void TApplication::Init(void) {
        printformat("TApplication: init.\n");
    }
    
    int TApplication::run(void) {
        
        running = true;
        desktop->draw();
        
        while (running) {
            desktop->menuBar->draw();
        }
        return 0;
    }
}   // namespace: tvision
