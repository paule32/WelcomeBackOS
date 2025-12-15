#pragma once

# include "stdint.h"

static inline void sti() { asm volatile ( "sti" ); }    // Enable interrupts
static inline void cli() { asm volatile ( "cli" ); }    // Disable interrupts
static inline void nop() { asm volatile ( "nop" ); }    // Do nothing

static inline UINT inportb(UINT port) {
	UINT ret_val;
	__asm__ volatile ("inb %w1,%b0"	: "=a"(ret_val)	: "d"(port));
	return ret_val;
}

static inline void outportb(UINT port, UINT val) {
    __asm__ volatile ("outb %b0,%w1" : : "a"(val), "d"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(val) : "dN"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" :: "a"(val), "dN"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t val;
    __asm__ __volatile__ ("inw %1, %0" : "=a"(val) : "dN"(port));
    return val;
}
static inline uint32_t inl(uint32_t port) {
    uint32_t val;
    __asm__ __volatile__ ("inl %1, %0" : "=a"(val) : "dN"(port));
    return val;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ __volatile__ ("outw %0, %1" :: "a"(val), "dN"(port));
}
static inline void outl(uint16_t port, uint32_t val) {
    __asm__ __volatile__ ("outl %0, %1" :: "a"(val), "dN"(port));
}

// video.c
#ifdef __cplusplus
extern "C" {
#endif
extern void k_clear_screen();
extern void settextcolor(unsigned char forecolor, unsigned char backcolor);
extern void putch(char c);
extern void puts(char* text);
extern void scroll();
extern void k_printf(char* message, UINT line, unsigned char attribute);
extern void set_cursor(unsigned char x, unsigned char y);
extern void update_cursor();
extern void move_cursor_right();
extern void move_cursor_left();
extern void move_cursor_home();
extern void move_cursor_end();
extern void save_cursor();
extern void restore_cursor();
extern void printformat (char *args, ...);
#ifdef __cplusplus
};
#endif

// kheap.c
#ifdef __cplusplus
extern "C" {
#endif
extern void*   kmemcpy(void* dst, const void* src, uint32_t n);
extern void*   kmemset(void* dst, int value, uint32_t n);
extern USHORT* kmemsetw(USHORT* dest, USHORT val, size_t count);
#ifdef __cplusplus
};
#endif

// util.c
#ifdef __cplusplus
extern "C" {
#endif
extern void kitoa(int value, char* valuestring);
extern void ki2hex(UINT val, char* dest, int len);
#ifdef __cplusplus
};
#endif

extern void *mmio_map(uint32_t phys, uint32_t size);

extern int check_atapi (void);
extern int check_ahci  (void);

extern int ahci_init   (void);
extern int ahci_probe_ports(void);

extern int atapi_read_sectors(uint32_t lba, uint32_t count, void *buffer);
extern int  sata_read_sectors(uint32_t lba, uint32_t count, void *buffer);

extern int cd_test_iso9660(void);
