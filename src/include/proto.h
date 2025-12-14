#pragma once

# include "stdint.h"

inline UINT inportb(UINT port) {
	UINT ret_val;
	__asm__ volatile ("inb %w1,%b0"	: "=a"(ret_val)	: "d"(port));
	return ret_val;
}

inline void outportb(UINT port, UINT val) {
    __asm__ volatile ("outb %b0,%w1" : : "a"(val), "d"(port));
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
