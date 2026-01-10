// ----------------------------------------------------------------------------
// \file  TCommandIDs.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __TURBOVISION_TCOMMANDIDS_H__
#define __TURBOVISION_TCOMMANDIDS_H__

enum command_t {
    cmNothing    = 0,
    cmQuit       = 1,
    cmMenu       = 2,
    cmHelp       = 3,
    cmNext       = 4,
    cmPrev       = 5,
    cmClose      = 6,

    cmCopy       = 20,
    cmPaste      = 21,
    cmCut        = 22,

    cmCustomBase = 1000
};

#endif  // __TURBOVISION_TCOMMANDIDS_H__
