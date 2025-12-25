// ---------------------------------------------------------------------------
// \file  crt0.S
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------

/* crt0.S - i386 userland start */
.global _start
.extern main
.extern sys_exit

_start:
    /* optional: Stack 16-byte align (nice-to-have) */
    and esp, 0xFFFFFFF0

    /* main(argc=0, argv=NULL) */
    push 0
    push 0
    call main

    /* Rückgabe von main als exit code */
    push eax
    call sys_exit

.hang:
    jmp .hang
