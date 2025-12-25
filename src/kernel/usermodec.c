# include "stdint.h"
# include "syscall.h"
# include "kheap.h"
# include "proto.h"

extern void enter_shell(void);

// einfacher Syscall-Wrapper
static inline void syscall_putchar(char c)
{
    __asm__ volatile(
        "int $0x80"
        :
        : "a"( (uint32_t)1 ),     // EAX = SYSCALL_PUTCHAR
          "b"( (uint32_t)(uint8_t)c ) // EBX = Zeichen
        : "memory"
    );
}

void user_mode_main(void)
{
    printformat("[user] Hello from ring 3!\n");
    enter_shell();
}

// User-Stack (4 KiB)
uint8_t  user_stack[4096];
uint32_t user_stack_top = (uint32_t)(user_stack + sizeof(user_stack));
