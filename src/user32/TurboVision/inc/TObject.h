// ----------------------------------------------------------------------------
// \file  TObject.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
//
// \note  Turbo Vision - Version 2.0
//        Copyright (c) 1994 by Borland International
//        All Rights Reserved.
// ----------------------------------------------------------------------------
#ifndef __TVISION_TOBJECT_H__
#define __TVISION_TOBJECT_H__

# include "stdint.h"

namespace tvision {
    class TObject {
    public:
        virtual ~TObject() {}

        static  void destroy(TObject*);
        virtual void shutDown();
    private:
    };

    inline void TObject::destroy(TObject *o)
    {
        if(o != 0) o->shutDown();
        delete o;
    }

}

#endif  // __TVISION_TOBJECT_H__
