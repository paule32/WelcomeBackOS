// ---------------------------------------------------------------------------
// \file  program_1.c – A simple freestanding C-Kernel
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
#include "os.h"
#include "kheap.h"
#include "task.h"
#include "initrd.h"
#include "syscall.h"
#include "shared_pages.h"
#include "flpydsk.h"

void user_program_1(void)
{
    k_clear_screen();
    settextcolor(14, 1);
    set_cursor(0, 0);
    for (int i = 0; i < 79 * 25 + 24; ++i) {
        putch('O');
    }
    set_cursor(0, 0);
}
