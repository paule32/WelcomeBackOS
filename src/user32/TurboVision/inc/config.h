// ----------------------------------------------------------------------------
// \file  config.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
//
// \note  Turbo Vision - Version 2.0
//        Copyright (c) 1994 by Borland International
//        All Rights Reserved.
// ----------------------------------------------------------------------------
#ifndef  __CONFIG_H__
# define __CONFIG_H__

# include "stdint.h"

const int eventQSize = 16;
const int maxCollectionSize = (int)(( (unsigned long) UINT_MAX - 16)/sizeof( void * ));

const int maxViewWidth = 132;

const int maxFindStrLen    = 80;
const int maxReplaceStrLen = 80;

const int minPasteEventCount = 3;

const int maxCharSize = 4;  // A UTF-8 encoded character is up to 4 bytes long
#endif  // __CONFIG_H__
