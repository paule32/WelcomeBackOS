// ----------------------------------------------------------------------------
// \file  shell32.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# define IMPORT

# include "stdint.h"
# include "proto.h"

# include "ksymbol_table.h"

extern     void kernel_symbols_init(void);
extern     void clear_screen(void);

extern "C" void app_run_demo(void);
extern "C" void ExitProcess (int );

extern "C" void shell_main(
    uint32_t         sym_count,
    kernel_symbol_t* sym_table) {
    
    kernel_symbols_count = sym_count;
    kernel_symbols       = sym_table;

    kernel_symbols_init();
    clear_screen();
    
    // Testmarker, bevor wir springen:
    volatile char* VGA = (volatile char*)0xB8000;
    VGA[0] = 'K'; VGA[1] = 0x0F;
    VGA[2] = 'U'; VGA[3] = 0x0F;
    VGA[4] = 'L'; VGA[5] = 0x0F;
    
    app_run_demo();
    
    ExitProcess(2);
    for(;;);
}
