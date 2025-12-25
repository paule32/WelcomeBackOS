#ifndef __PROTO_H__
#define __PROTO_H__
#pragma once

# include "stdint.h"

#pragma pack(push, 1)
typedef struct {
    uint16_t e_magic;      // "MZ" = 0x5A4D
    uint16_t e_cblp;
    uint16_t e_cp;
    uint16_t e_crlc;
    uint16_t e_cparhdr;
    uint16_t e_minalloc;
    uint16_t e_maxalloc;
    uint16_t e_ss;
    uint16_t e_sp;
    uint16_t e_csum;
    uint16_t e_ip;
    uint16_t e_cs;
    uint16_t e_lfarlc;
    uint16_t e_ovno;
    uint16_t e_res[4];
    uint16_t e_oemid;
    uint16_t e_oeminfo;
    uint16_t e_res2[10];
    int32_t  e_lfanew;     // Offset zum PE-Header
} dos_header_t;

typedef struct {
    uint32_t Signature;     // "PE\0\0" = 0x00004550
} pe_signature_t;

typedef struct {
    uint16_t Machine;
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
} coff_header_t;

typedef struct {
    uint16_t Magic;                     // 0x10B = PE32
    uint8_t  MajorLinkerVersion;
    uint8_t  MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;       // RVA
    uint32_t BaseOfCode;                // RVA
    uint32_t BaseOfData;                // RVA

    uint32_t ImageBase;
    uint32_t SectionAlignment;
    uint32_t FileAlignment;
    uint16_t MajorOperatingSystemVersion;
    uint16_t MinorOperatingSystemVersion;
    uint16_t MajorImageVersion;
    uint16_t MinorImageVersion;
    uint16_t MajorSubsystemVersion;
    uint16_t MinorSubsystemVersion;
    uint32_t Win32VersionValue;
    uint32_t SizeOfImage;
    uint32_t SizeOfHeaders;
    uint32_t CheckSum;
    uint16_t Subsystem;
    uint16_t DllCharacteristics;
    uint32_t SizeOfStackReserve;
    uint32_t SizeOfStackCommit;
    uint32_t SizeOfHeapReserve;
    uint32_t SizeOfHeapCommit;
    uint32_t LoaderFlags;
    uint32_t NumberOfRvaAndSizes;
    // Danach folgen DataDirectories – die ignorieren wir erstmal
} optional_header_t;

typedef struct {
    char     Name[8];
    uint32_t VirtualSize;
    uint32_t VirtualAddress;      // RVA
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLinenumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers;
    uint32_t Characteristics;
} section_header_t;

#pragma pack(pop)

typedef struct {
    uint32_t entry_point;   // absolute Adresse
    uint32_t image_base;    // basisadresse
    uint32_t image_size;
} pe_image_t;

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
static inline uint32_t inl(uint16_t port)
{
    uint32_t val;
    __asm__ __volatile__ ("inl %1, %0"
                          : "=a"(val)
                          : "Nd"(port));
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
extern void kprintf(char* message, UINT line, unsigned char attribute);
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

// util.c
#ifdef __cplusplus
extern "C" {
#endif
extern void kitoa(int value, char* valuestring);
extern void ki2hex(UINT val, char* dest, int len);

extern void *mmio_map(uint32_t phys, uint32_t size);
#ifdef __cplusplus
};
#endif

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
extern int check_atapi (void);
extern int check_ahci  (void);

extern int ahci_init   (void);
extern int ahci_probe_ports(void);

extern int atapi_read_sectors(uint32_t lba, uint32_t count, void *buffer);
extern int  sata_read_sectors(uint32_t lba, uint32_t count, void *buffer);

extern int cd_test_iso9660(void);

extern int pe_load_image(const uint8_t* file, uint32_t size, pe_image_t* out);

typedef struct {
    uint32_t base;
    uint32_t length;
    uint32_t type;   // 1 = usable
}   mem_map_entry_t;

extern          mem_map_entry_t* mem_map;
extern uint32_t mem_map_length; // Anzahl Einträge
extern uint32_t max_mem;        // oberstes Ende des nutzbaren physikalischen RAMs
#ifdef __cplusplus
};
#endif  // __cplusplus
#endif  // __PROTO_H__
