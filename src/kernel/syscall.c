// ----------------------------------------------------------------------------
// \file  syscall.c
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
# include "stdint.h"
# include "proto.h"
# include "kheap.h"

# include "syscall.h"
# include "idt.h"
# include "isr.h"

syscall_fn_t g_syscalls[SYS_MAX] = {
    [SYS_PRINT]     = sys_print,
    [SYS_ALLOC]     = sys_alloc,
    [SYS_FREE]      = sys_free,
};

const KApi_v1 g_kapi_v1 = {
    .magic = KAPI_MAGIC,
    .version = 1,
    .size = sizeof(KApi_v1),
    .flags = 0,

    .sys_print = SYS_PRINT,
    .sys_alloc = SYS_ALLOC,
    .sys_free  = SYS_FREE
};

size_t strlcpy(char *dst, const char *src, size_t dst_size)
{
    const char *s = src;
    size_t src_len = 0;
    while (*s++) src_len++;

    if (dst_size == 0) return src_len;
    size_t to_copy = (src_len >= dst_size) ? (dst_size - 1) : src_len;

    for (size_t i = 0; i < to_copy; i++)
        dst[i] = src[i];

    dst[to_copy] = '\0';
    return src_len;
}

static int32_t sys_print(regs_t* reg)
{
    volatile char* VGA = (volatile char*)0xB8000;
    VGA[10] = 'O'; VGA[11] = 0x0F;
    VGA[12] = 'P'; VGA[13] = 0x0F;
    VGA[14] = 'A'; VGA[15] = 0x0F;

    if (reg->ebx != 0) {
        char*  s = (char*)kmalloc(200);
        strlcpy(s, (char*)(uintptr_t)reg->ebx, 200);
        printformat("--> 0x%x\n", s);
        printformat("--> %s -\n", s);
        
        uint32_t addr32 = (uint32_t)reg->ebx;
        char *p = (char *)(uintptr_t)addr32;
        printformat("--> 0x%x\n", p);
        printformat("--> %s -\n", p);
        kfree(s);
    }   else {
        printformat("EBX = null.\n");
    }
    return 0;
}

static int32_t sys_alloc(regs_t* reg)
{
    uint32_t size = reg->ebx;
    void*p = (void*)kmalloc(size);
    if (!p) return 0;
    return (int32_t)(uintptr_t)p;
}

static int32_t sys_free(regs_t* reg)
{
    void* p = (void*)reg->ebx;
    kfree(p);
    return 0;
}

static volatile char* const VGA = (volatile char*)0xB8000;

static void vga_putc(char c)
{
    static int pos = 160; // zweite Zeile (je 2 Byte pro Zeichen)
    if (pos >= 80 * 2 * 10) pos = 160; // primitive "Zeilenbegrenzung"

    VGA[pos]     = c;
    VGA[pos + 1] = 0x07;
    pos += 2;
}

void syscall_init(void)
{
    // Hier wäre später Platz für eine Syscall-Tabelle etc.
    // aktuell nur Platzhalter, eigentliche IDT-Eintragung erfolgt bereits in idt_init()
}

void syscall_dispatch(regs_t* reg)
{
    // Konvention: EAX = Syscall ID, EBX, ECX, EDX = Parameter
    uint32_t num = reg->eax;

    int32_t ret = g_syscalls[num](reg);
    reg->eax = (uint32_t)ret;
}
#if 0
    switch (num) {
        case SYSCALL_PUTCHAR:
            vga_putc((char)r->ebx);   // Beispiel: EBX = Zeichen
            break;

        default:
            // unbekannter Syscall
            vga_putc('?');
            break;
    }
}
#endif



#if 0
// some functions declared extern in os.h
// rest of functions must be declared here:
extern void f4();

DEFN_SYSCALL1( puts,                       0, unsigned char*               )
DEFN_SYSCALL1( putch,                      1, unsigned char                )
DEFN_SYSCALL2( settextcolor,               2, unsigned char, unsigned char )
DEFN_SYSCALL0( getpid,                     3                               )
DEFN_SYSCALL0( nop,                        4                               )
DEFN_SYSCALL0( switch_context,             5                               )
DEFN_SYSCALL0( k_checkKQ_and_print_char,   6                               )
DEFN_SYSCALL0( k_checkKQ_and_return_char,  7                               )

#define NUM_SYSCALLS 8

static void* syscalls[NUM_SYSCALLS] =
{
    &puts,
    &putch,
    &settextcolor,
    &getpid,
    &nop,
    &switch_context,
    &k_checkKQ_and_print_char,
    &k_checkKQ_and_return_char
};

void syscall_handler(struct regs* r)
{
    // Firstly, check if the requested syscall number is valid. The syscall number is found in EAX.
    if( r->eax >= NUM_SYSCALLS )
        return;

    void* addr = syscalls[r->eax]; // Get the required syscall location.

    // We don't know how many parameters the function wants, so we just push them all onto the stack in the correct order.
    // The function will use all the parameters it wants, and we can pop them all back off afterwards.
    int ret;
    asm volatile (" \
      push %1; \
      push %2; \
      push %3; \
      push %4; \
      push %5; \
      call *%6; \
      add $20, %%esp; \
    " : "=a" (ret) : "r" (r->edi), "r" (r->esi), "r" (r->edx), "r" (r->ecx), "r" (r->ebx), "r" (addr));
    r->eax = ret;
}
#endif
