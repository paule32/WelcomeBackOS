// ----------------------------------------------------------------------------
// \file  shell32.cc
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# define IMPORT

# include "stdint.h"
# include "proto.h"

# include "pe_loader.h"
# include "syscall.h"

extern     void clear_screen(unsigned char, unsigned char);

extern "C" void app_run_demo(void);
extern "C" void ExitProcess (int );

typedef struct KApiUser {
    KApi_v1* k;
    syscall_fn_t* g_syscall_fun;
} KApiUser;

const  pe_user_image_t* pe_image;

static KApiUser api;
static inline void k_print(KApiUser* api, const char* s) {
    if (!api) {
    }   else {
        volatile char* VGA = (volatile char*)0xB8000;
        VGA[26] = 'P'; VGA[27] = 0x0F;
        VGA[28] = 'R'; VGA[29] = 0x0F;
        VGA[30] = 'U'; VGA[31] = 0x0F;
    }
    
    uint32_t addr = (uint32_t)(uintptr_t)s; 
    
    regs_t reg;
    reg.eax = SYS_PRINT;
    reg.ebx = addr;

    int32_t ret = api->g_syscall_fun[reg.eax](&reg);
    reg.eax = (uint32_t)ret;
}

extern "C" void shell_main(uint32_t t, const pe_user_image_t* img, struct KApi_v1* a, syscall_fn_t* scall)
{
    api.k = a;
    api.g_syscall_fun = scall;
    pe_image   = img;
    
    if (!a) {
        volatile char* VGA = (volatile char*)0xB8000;
        VGA[0] = 'K'; VGA[1] = 0x0F;
        VGA[2] = 'A'; VGA[3] = 0x0F;
        VGA[4] = 'L'; VGA[5] = 0x0F;
    }   else {
        volatile char* VGA = (volatile char*)0xB8000;
        VGA[0] = 'Q'; VGA[1] = 0x0F;
        VGA[2] = 'U'; VGA[3] = 0x0F;
        VGA[4] = 'K'; VGA[5] = 0x0F;
    }
    
    k_print(&api, "Hallo Application\n");

//    clear_screen(0x1F, 0xB1);
    /*
    // Testmarker, bevor wir springen:
    volatile char* VGA = (volatile char*)0xB8000;
    VGA[0] = 'K'; VGA[1] = 0x0F;
    VGA[2] = 'U'; VGA[3] = 0x0F;
    VGA[4] = 'L'; VGA[5] = 0x0F;
    */
    
//    app_run_demo();
    
    ExitProcess(2);
    for(;;);
}
