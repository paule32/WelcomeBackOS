/// #define _DIAGNOSIS_ // activates prints to the screen about memory use

#ifndef OS_H
#define OS_H

#define TRUE		1
#define FALSE		0

typedef unsigned int   size_t;
typedef unsigned int   UINT;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;

extern ULONG placement_address;

#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))
extern void panic_assert(const char* file, ULONG line, const char* desc);

/* This defines the operatings system common data area */

#define KQSIZE    20 // size of key queue

typedef struct oda
{
    // Hardware Data
    ULONG COM1, COM2, COM3, COM4; // address
    ULONG LPT1, LPT2, LPT3, LPT4; // address
    ULONG Memory_Size;            // Memory size in Byte

    // Key Queue
    unsigned char  KEYQUEUE[KQSIZE];   // circular queue buffer
    unsigned char* pHeadKQ;            // pointer to the head of valid data
    unsigned char* pTailKQ;            // pointer to the tail of valid data
    ULONG  KQ_count_read;      // number of data read from queue buffer
    ULONG  KQ_count_write;     // number of data put into queue buffer

    //tasking
    unsigned char  ts_flag;            // 0: taskswitch off  1: taskswitch on
}oda_t;

// operatings system common data area
extern oda_t ODA;
extern oda_t* pODA;

typedef struct Mem_Chunk_struct
 {
   ULONG base_lo;
   ULONG base_hi;
   ULONG length_lo;
   ULONG length_hi;
   ULONG type;
   ULONG extended;
 }Mem_Chunk_t;


/* This defines what the stack looks like after an ISR was running */
typedef struct regs
{
    ULONG gs, fs, es, ds;
    ULONG edi, esi, ebp, ebx, edx, ecx, eax;
    ULONG int_no, err_code;
    ULONG eip, cs, eflags, useresp, ss;
}registers_t;

// video.c
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

// timer.c
extern void timer_handler(struct regs* r);
extern void timer_wait (ULONG ticks);
extern void sleepSeconds (ULONG seconds);
extern void sleepMilliSeconds (ULONG ms);
extern void systemTimer_setFrequency( ULONG freq );
extern void timer_install();
extern void timer_uninstall();

// keyboard.c
extern void keyboard_init();
extern unsigned char FetchAndAnalyzeScancode();
extern unsigned char ScanToASCII();
extern void keyboard_handler(struct regs* r);
extern int k_checkKQ_and_print_char();
extern unsigned char k_checkKQ_and_return_char();

// util.c
extern void initODA();
extern void outportb(UINT port, UINT val);
extern UINT inportb(UINT port);
extern ULONG fetchESP();
extern ULONG fetchEBP();
extern ULONG fetchSS();
extern ULONG fetchCS();
extern ULONG fetchDS();
extern void k_memshow(void* start, size_t count);
extern void* k_memset(void* dest, char val, size_t count);
extern USHORT* k_memsetw(USHORT* dest, USHORT val, size_t count);
extern void* k_memcpy(void* dest, const void* src, size_t count);
extern size_t k_strlen(const char* str);
extern int k_strcmp( const char* s1, const char* s2 );
extern char* k_strcpy(char* dest, const char* src);
extern char* k_strncpy(char* dest, const char* src, size_t n);
extern char* k_strcat(char* dest, const char* src);
extern void reboot();
extern void cli();
extern void sti();
extern void nop();
extern void k_itoa(int value, char* valuestring);
extern void k_i2hex(UINT val, char* dest, int len);
extern void float2string(float value, int decimal, char* valuestring);

// gtd.c itd.c irq.c isrs.c
extern void gdt_set_gate(int num, ULONG base, ULONG limit, unsigned char access, unsigned char gran);
extern void gdt_install();
extern void idt_set_gate(unsigned char num, ULONG base, USHORT sel, unsigned char flags);
extern void idt_install();
extern void isrs_install();
extern void irq_install();

extern void timer_install();
extern void keyboard_install();

extern ULONG fault_handler(ULONG esp);
extern ULONG irq_handler(ULONG esp);
extern void irq_install_handler(int irq, void (*handler)(struct regs* r));
extern void irq_uninstall_handler(int irq);
extern void irq_remap(void);


// math.c
extern int k_abs(int i);
extern int k_power(int base,int n);

// paging.c
extern void paging_install();
extern ULONG k_malloc(ULONG size, unsigned char align, ULONG* phys);
extern void analyze_frames_bitset(ULONG sec);
extern ULONG show_physical_address(ULONG virtual_address);
extern void analyze_physical_addresses();

#endif
