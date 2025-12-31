// ----------------------------------------------------------------------------
// \file  ctor.cc
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------

extern "C" volatile int g_ctor_ran = 0;

extern "C" {
    typedef void (*ctor_t)();
    
    extern ctor_t _init_array_start[];
    extern ctor_t _init_array_end[];
}

extern "C" void call_global_ctors() {
    g_ctor_ran = 1234;
    for (ctor_t* p = _init_array_start; p != _init_array_end; ++p)
    (*p)();
}
