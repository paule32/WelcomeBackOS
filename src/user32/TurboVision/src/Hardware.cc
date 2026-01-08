// ----------------------------------------------------------------------------
// \file  Hardware.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
//
// \note  Turbo Vision - Version 2.0
//        Copyright (c) 1994 by Borland International
//        All Rights Reserved.
// ----------------------------------------------------------------------------
# define Uses_THardwareInfo

# include "tv.h"

uint16_t THardwareInfo::biosSel  = 0x0040;
uint16_t THardwareInfo::monoSel  = 0xB000;
uint16_t THardwareInfo::colorSel = 0xB800;

THardwareInfo::THardwareInfo()
{
    
}

THardwareInfo::~THardwareInfo()
{
    
}
