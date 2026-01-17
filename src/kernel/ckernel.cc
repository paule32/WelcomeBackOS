// ---------------------------------------------------------------------------
// \file  kernel.c – A simple freestanding C-Kernel
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
# include "config.h"

# include "paging.h"
# include "kheap.h"
# include "gdt.h"
# include "idt.h"
# include "isr.h"
# include "task.h"
# include "syscall.h"
# include "usermode.h"
# include "iso9660.h"
# include "proto.h"

# define DESKTOP
# include "vga.h"

extern void* krealloc(void* ptr, uint32_t new_size);

extern int atapi_read_sectors(uint32_t lba, uint32_t count, void *buffer);
extern int  sata_read_sectors(uint32_t lba, uint32_t count, void *buffer);

extern "C" void clear_screen(unsigned char, unsigned char);
extern "C" void printformat(char*, ...);

extern "C" void detect_memory(void);
extern "C" void enter_usermode(void);
extern "C" void irq_init(void);
extern "C" void idt_init(void);
extern "C" void isr_init(void);
extern "C" void gdt_init();
extern "C" void syscall_init(void);
extern "C" void tasking_init(void);

extern "C" void call_global_ctors(void);

extern "C" void* __gxx_personality_v0(){return (void*)0;}
int  graph_mode = 0;

extern uint32_t __end;

# define KSTACK_SIZE  (64*1024)

uint32_t kernel_stack_top    = 0x00180000;
uint32_t kernel_stack_bottom = kernel_stack_top - KSTACK_SIZE;

extern uint8_t set1_font8x8[256][8];
extern uint8_t set2_font8x8[256][8];

static inline void putpixel8(uint8_t* fb, int pitch, int x, int y, uint8_t c){
    fb[y*pitch + x] = c;
}
static void fillrect(uint8_t* fb, int pitch, int x,int y,int w,int h, uint8_t c){
    for(int yy=0; yy<h; yy++)
        for(int xx=0; xx<w; xx++)
            putpixel8(fb,pitch,x+xx,y+yy,c);
}
static void rect(uint8_t* fb, int pitch, int x,int y,int w,int h, uint8_t c){
    for(int i=0;i<w;i++){ putpixel8(fb,pitch,x+i,y,c); putpixel8(fb,pitch,x+i,y+h-1,c); }
    for(int i=0;i<h;i++){ putpixel8(fb,pitch,x,y+i,c); putpixel8(fb,pitch,x+w-1,y+i,c); }
}

static void draw_glyph8x8(uint8_t* fb,int pitch,int x,int y,
                          const uint8_t g[8], uint8_t fg, uint8_t bg)
{
    for(int row=0; row<8; row++){
        uint8_t bits = g[row];
        for(int col=0; col<8; col++){
            uint8_t cc = (bits & (0x80 >> col)) ? fg : bg;
            putpixel8(fb,pitch,x+col,y+row,cc);
        }
    }
}

// 8x8 glyph -> 12x12
static void draw_glyph8x8_to_12x12(
    uint8_t* fb, int pitch,
    int x, int y,
    const uint8_t glyph[8],
    uint8_t fg, uint8_t bg)
{
    for (int ty = 0; ty < 12; ty++) {
        int sy = (ty * 8) / 12;          // 0..7
        uint8_t bits = glyph[sy];
        for (int tx = 0; tx < 12; tx++) {
            int sx = (tx * 8) / 12;      // 0..7
            uint8_t mask = (uint8_t)(0x80u >> sx);
            uint8_t c = (bits & mask) ? fg : bg;
            putpixel8(fb, pitch, x + tx, y + ty, c);
        }
    }
}
void draw_layout_640x400(uint8_t* fb, int pitch, const uint8_t font[256][8]) {
    // colors: choose palette indices as you like
    const uint8_t BG = 9;   // e.g. dark-ish blue (depends on palette)
    const uint8_t FRAME1 = 14; // yellow
    const uint8_t FRAME2 = 3;  // cyan-ish

    fillrect(fb,pitch,0,0,640,400,BG);

    // outer frame
    //rect(fb,pitch,10,10,620,380,FRAME1);
    //rect(fb,pitch,12,12,616,376,FRAME2);

    // viewport (320x200 centered)
    const int cell = 12;
    const int text_w = 40 * cell;   // 480
    const int text_h = 25 * cell;   // 300
    const int vx = (640 - text_w) / 2;  // 80
    const int vy = (400 - text_h) / 2;  // 50
    
    //int vx = 85, vy = 58;
    
    //rect(fb,pitch,vx-2,vy-2,474,284,FRAME1);
    fillrect(fb,pitch,vx,vy,470,280,BG);

/*
    // demo: write 40x25 chars (exactly fits 320x200)
    for(int cy=0; cy<25; cy++){
        for(int cx=0; cx<40; cx++){
            uint8_t ch = (uint8_t)(cx + cy*40);
            draw_glyph8x8(fb,pitch, vx + cx*8, vy + cy*8, font[ch], 15, BG);
        }
    }
*/
# define X_START 76
# define Y_START 48

uint16_t tx = X_START, ty = Y_START;
uint16_t cx =  1;
const uint8_t*ch;

for (int l = 0; l < 256; l++) {
    ch = set1_font8x8[l];

    draw_glyph8x8_to_12x12(
    fb, pitch,
    tx, ty, ch,
    1, 11);
    
    tx  = X_START + cx * 12;
    cx += 1;

    if (cx > 41) {
        ty += 12;
        tx  = X_START;
        cx  =  1;
    }
}
tx  = X_START;
for (int l = 0; l < 256; l++) {
    ch = set2_font8x8[l];

    draw_glyph8x8_to_12x12(
    fb, pitch,
    tx, ty, ch,
    1, 11);
    
    tx  = X_START + cx * 12;
    cx += 1;

    if (cx > 41) {
        ty += 12;
        tx  = X_START;
        cx  =  1;
    }
}
tx  = X_START;
for (int l = 0; l < 256; l++) {
    ch = set1_font8x8[l];

    draw_glyph8x8_to_12x12(
    fb, pitch,
    tx, ty, ch,
    1, 11);
    
    tx  = X_START + cx * 12;
    cx += 1;

    if (cx > 41) {
        ty += 12;
        tx  = X_START;
        cx  =  1;
    }
}
tx  = X_START;
for (int l = 0; l < 256; l++) {
    ch = set2_font8x8[l];

    draw_glyph8x8_to_12x12(
    fb, pitch,
    tx, ty, ch,
    1, 11);
    
    tx  = X_START + cx * 12;
    cx += 1;

    if (cx > 41) {
        ty += 12;
        tx  = X_START;
        cx  =  1;
    }
}

/*
    for (int cy = 0; cy < 25; cy++) {
        for (cx = 0; cx < 40; cx++) {
            uint8_t ch = (uint8_t)(cy*40 + cx);           // PETSCII byte
            draw_glyph8x8_to_12x12(
            fb, pitch,
            vx + cx*12,
            vy + cy*12,
            font[0],
            0, 14);
        }
    }*/
}

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

typedef unsigned int addr_t;   // 32-bit

static inline void fillSpan16(volatile uint16_t *dst, int count, uint16_t color)
{
    __asm__ __volatile__ (
        "cld\n\t"
        "rep stosw"
        : "+D"(dst), "+c"(count)
        : "a"(color)
        : "memory"
    );
}

void fillRect16_fast(int x, int y, int w, int h, uint16_t color565)
{
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }

    if (x + w > lfb_xres) w = lfb_xres - x;
    if (y + h > lfb_yres) h = lfb_yres - y;
    if (w <= 0 || h <= 0) return;

    // lfb_base MUSS eine Adresse sein, keine Pointer-Arithmetik mit falschem Typ
    addr_t base = (addr_t)lfb_base;      // lfb_base kann int/ptr sein – wir machen eine Zahl draus
    addr_t off  = (addr_t)(y * lfb_pitch + x * 2);

    volatile uint8_t *row8 = (volatile uint8_t *)(base + off);

    for (int j = 0; j < h; j++) {
        fillSpan16((volatile uint16_t*)row8, w, color565);
        row8 += lfb_pitch;              // pitch ist BYTES!
    }
}
extern "C" int ami_main()
{
    idt_init();
 
    paging_init();
    kheap_init();

    //detect_memory();          // setzt max_mem
    
    uint32_t stack_top = kernel_stack_top;
    uint32_t stack_bottom = stack_top - (64*1024);

    uint32_t reserved = ((uint32_t)&__end + 0xFFF) & ~0xFFF;
    if (reserved < kernel_stack_top) reserved = kernel_stack_top; // Stack schützen
    page_init(reserved);
    
    // 3) aktuellen Stack als esp0 für TSS verwenden
    uint32_t esp;
    __asm__ volatile("mov %%esp, %0" : "=r"(esp));
    gdt_init();

    isr_init();
    irq_init();

    syscall_init();
    tasking_init();
    
    const vbe_info_t* mi = ((const vbe_info_t*)0x00002000);

    lfb_base  = mi->phys_base;
    lfb_pitch = mi->pitch; 
    lfb_bpp   = mi->bpp ;
    
    lfb_xres  = mi->xres;
    lfb_yres  = mi->yres;
    
    size_t fb_bytes = (size_t)lfb_pitch * (size_t)lfb_yres;

    // WICHTIG: phys -> virt mappen
    uintptr_t lfb_virt = (uintptr_t)mmio_map(lfb_base, (uint32_t)fb_bytes);

    // Ab jetzt IMMER die virtuelle Adresse zum Schreiben benutzen!
    lfb_base = (uint32_t)lfb_virt;
    
    
    fillRect16_fast(0,0,lfb_xres,lfb_yres,rgb565(255,255,255));
    for (;;);
}
// c64 kernel
extern "C" int c64_main()
{
    idt_init();
 
    paging_init();
    kheap_init();

    //detect_memory();          // setzt max_mem
    
    uint32_t stack_top = kernel_stack_top;
    uint32_t stack_bottom = stack_top - (64*1024);

    uint32_t reserved = ((uint32_t)&__end + 0xFFF) & ~0xFFF;
    if (reserved < kernel_stack_top) reserved = kernel_stack_top; // Stack schützen
    page_init(reserved);
    
    // 3) aktuellen Stack als esp0 für TSS verwenden
    uint32_t esp;
    __asm__ volatile("mov %%esp, %0" : "=r"(esp));
    gdt_init();

    isr_init();
    irq_init();

    syscall_init();
    tasking_init();
    
    const vbe_info_t* mi = ((const vbe_info_t*)0x00002000);

    lfb_base  = mi->phys_base;
    lfb_pitch = mi->pitch; 
    lfb_bpp   = mi->bpp ;
    
    lfb_xres  = mi->xres;
    lfb_yres  = mi->yres;
    
    size_t fb_bytes = (size_t)lfb_pitch * (size_t)lfb_yres;

    // WICHTIG: phys -> virt mappen
    uintptr_t lfb_virt = (uintptr_t)mmio_map(lfb_base, (uint32_t)fb_bytes);

    // Ab jetzt IMMER die virtuelle Adresse zum Schreiben benutzen!
    lfb_base = (uint32_t)lfb_virt;
    putpixel8(
        reinterpret_cast<uint8_t*>(lfb_base),
        lfb_pitch, 10, 10, 14);
    putpixel8(
        reinterpret_cast<uint8_t*>(lfb_base),
        lfb_pitch, 11, 10, 14);
    putpixel8(
        reinterpret_cast<uint8_t*>(lfb_base),
        lfb_pitch, 10, 11, 14);
    draw_layout_640x400(reinterpret_cast<uint8_t*>(lfb_base), lfb_pitch, set1_font8x8);
    for (;;);
}

// text kernel
extern "C" int txt_main()
{
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    vga[0] = 0x0F00 | 'K';
    graph_mode = 1;

    idt_init();
 
    paging_init();
    kheap_init();

    //detect_memory();          // setzt max_mem
    
    uint32_t stack_top = kernel_stack_top;
    uint32_t stack_bottom = stack_top - (64*1024);

    uint32_t reserved = ((uint32_t)&__end + 0xFFF) & ~0xFFF;
    if (reserved < kernel_stack_top) reserved = kernel_stack_top; // Stack schützen
    page_init(reserved);
    
    // 3) aktuellen Stack als esp0 für TSS verwenden
    uint32_t esp;
    __asm__ volatile("mov %%esp, %0" : "=r"(esp));
    gdt_init();

    isr_init();
    irq_init();

    syscall_init();
    tasking_init();

    // ------------------------------
    // global c++ constructor's init.
    // ------------------------------
    call_global_ctors();

    settextcolor(14,0);
    
    if (check_atapi() == 0) {
        // ATAPI (IDE) gefunden
        printformat("ATAPI: OK.\n");
    }   else {
        printformat("ATAPI: NOK\n");
        check_ahci();
    }
    
    if (iso_mount() != 0) {
        printformat("ISO mount Error.\n");
    }   else {
        printformat("ISO mount successfully.\n");
    }

    __asm__ volatile("sti");
    
    // Testmarker, bevor wir springen:
    volatile char* VGA = (volatile char*)0xB8000;
    VGA[0] = 'K'; VGA[1] = 0x0F;
    VGA[2] = 'U'; VGA[3] = 0x0F;

    enter_usermode();
    
    // Hierher kommt man normalerweise nicht mehr zurück
    for (;;) {
        asm volatile("hlt");
    }
    return 0;

    printformat("\nin text kernel\n");
    for (;;);
}

// graphics kernel
extern "C" int vid_main()
{
    graph_mode = 2;
    
    idt_init();
 
    paging_init();
    kheap_init();
 
    //detect_memory();          // setzt max_mem
    
    uint32_t stack_top = kernel_stack_top;
    uint32_t stack_bottom = stack_top - (64*1024);

    uint32_t reserved = ((uint32_t)&__end + 0xFFF) & ~0xFFF;
    if (reserved < kernel_stack_top) reserved = kernel_stack_top; // Stack schützen
    page_init(reserved);

    // 3) aktuellen Stack als esp0 für TSS verwenden
    uint32_t esp;
    //__asm__ volatile("mov %%esp, %0" : "=r"(esp));
    gdt_init();
    isr_init();
    irq_init();
    
    syscall_init();
    tasking_init();
    
    // ------------------------------
    // vesa 0x114: 800 x 600 x 16bpp
    // ------------------------------
    gfx_init();
    
    // ------------------------------
    // global c++ constructor's init.
    // ------------------------------
    call_global_ctors();
    
    //try {
        //gfx_printf("ctorflag = %d\n", 123);
        /*extern TCanvas canvas_desktop;
        extern "C" volatile int g_ctor_ran;
        gfx_printf("ctorflag=%d\n", g_ctor_ran);*/
    //}
    //catch (...) {
//        gfx_printf("xxxxxx\n");
//        for(;;);
//    }
    
    
    /*TCanvas *foo = new TCanvas();
    if (canvas_desktop.flag == 42) {
        for (;;);
    }*/
    
    settextcolor(14,0);
    
    if (check_atapi() == 0) {
        // ATAPI (IDE) gefunden
        gfx_printf("ATAPI: OK.\n");
    }   else {
        gfx_printf("ATAPI: NO\n");
        check_ahci();
    }

    if (iso_mount() != 0) {
        gfx_printf("ISO mount Error.\n");
    }   else {
        gfx_printf("ISO mount successfully.\n");
    }

    __asm__ volatile("sti");
    
    // Testmarker, bevor wir springen:
    /*volatile char* VGA = (volatile char*)0xB8000;
    VGA[0] = 'K'; VGA[1] = 0x0F;
    VGA[2] = 'U'; VGA[3] = 0x0F;*/

    enter_usermode();
    
    // Hierher kommt man normalerweise nicht mehr zurück
    for (;;) {
        asm volatile("hlt");
    }
    return 0;
}
