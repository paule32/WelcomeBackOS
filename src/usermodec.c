# include "stdint.h"
# include "syscall.h"
# include "kheap.h"

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

void start_shell_from_initrd(void)
{
    /*
    struct dirent* node = 0;
    
    fs_node_t* f = fs_find("shell.exe");   // je nach deinem FS-API
    if (!f) {
        // debug msg...
        return;
    }

    uint32_t size = f->length;
    uint8_t* buffer = (uint8_t*)kmalloc(size);

    fs_read(f, 0, size, (uint8_t*)buffer);  // einfach alles lesen

    pe_image_t img;
    int res = pe_load_image(buffer, size, &img);

    if (res != 0) {
        // debug: Fehlercode anzeigen
        return;
    }

    // Funktionstyp für Entry
    typedef void (*pe_entry_t)(void);
    pe_entry_t entry = (pe_entry_t)img.entry_point;

    // ZUERST NUR TEST: im Kernel-Kontext aufrufen
    entry();

    // Wird in der Regel nicht zurückkehren – shell_main hat eigene Schleife
    */
}

void user_mode_main(void)
{
    const char* s = "[user] Hello from ring 3!";
    for (int i = 0; s[i]; ++i) {
        syscall_putchar(s[i]);
    }

    // PE-Shell laden
    start_shell_from_initrd();
    
    while (1) {
        __asm__ volatile("hlt");
    }
}

// User-Stack (4 KiB)
uint8_t  user_stack[4096];
uint32_t user_stack_top = (uint32_t)(user_stack + sizeof(user_stack));
