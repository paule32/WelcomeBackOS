#ifndef __PE_LOADER_H__
#define __PE_LOADER_H__

# include "stdint.h"

# define IMAGE_DOS_SIGNATURE                0x5A4D  // MZ
# define IMAGE_NT_SIGNATURE                 0x00004550 // PE\0\0
# define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x10B

# define IMAGE_REL_BASED_HIGHLOW            3
# define IMAGE_NUMBEROF_DIRECTORY_ENTRIES   16

typedef struct {
    uint16_t e_magic;
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
    uint32_t e_lfanew;
} __attribute__((packed)) IMAGE_DOS_HEADER;

typedef struct {
    uint16_t Machine;
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
} __attribute__((packed)) IMAGE_FILE_HEADER;

typedef struct {
    uint32_t VirtualAddress;
    uint32_t Size;
} __attribute__((packed)) IMAGE_DATA_DIRECTORY;

typedef struct {
    uint32_t entry;      // EIP
    uint32_t image_base; // geladene Basis
    uint32_t image_size;
} pe_user_image_t;

typedef struct {
    uint32_t VirtualAddress;
    uint32_t SizeOfBlock;
} __attribute__((packed)) IMAGE_BASE_RELOCATION;

typedef struct {
    uint16_t Magic;                 // 0x10B = PE32
    uint8_t  MajorLinkerVersion;
    uint8_t  MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;   // RVA
    uint32_t BaseOfCode;            // RVA
    uint32_t BaseOfData;            // RVA (nur PE32)

    uint32_t ImageBase;             // bevorzugte Base (VA)
    uint32_t SectionAlignment;
    uint32_t FileAlignment;

    uint16_t MajorOperatingSystemVersion;
    uint16_t MinorOperatingSystemVersion;
    uint16_t MajorImageVersion;
    uint16_t MinorImageVersion;
    uint16_t MajorSubsystemVersion;
    uint16_t MinorSubsystemVersion;

    uint32_t Win32VersionValue;
    uint32_t SizeOfImage;           // Gesamtgröße im Speicher
    uint32_t SizeOfHeaders;         // Headergröße im File
    uint32_t CheckSum;

    uint16_t Subsystem;
    uint16_t DllCharacteristics;

    uint32_t SizeOfStackReserve;
    uint32_t SizeOfStackCommit;
    uint32_t SizeOfHeapReserve;
    uint32_t SizeOfHeapCommit;

    uint32_t LoaderFlags;
    uint32_t NumberOfRvaAndSizes;

    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} __attribute__((packed)) IMAGE_OPTIONAL_HEADER32;

typedef struct {
    uint32_t Signature;             // "PE\0\0" = 0x00004550
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} __attribute__((packed)) IMAGE_NT_HEADERS32;

typedef struct {
    uint8_t  Name[8];
    union {
        uint32_t PhysicalAddress;
        uint32_t VirtualSize;
    } Misc;
    uint32_t VirtualAddress;        // RVA
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLinenumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers;
    uint32_t Characteristics;
} __attribute__((packed)) IMAGE_SECTION_HEADER;

// Ein Eintrag: Funktionsname + Offset (z.B. RVA oder bereits VA)
typedef struct {
    char    *name;     // dynamisch kopiert
    uint32_t offset;   // z.B. RVA (EntryPoint der Funktion) oder VA
} pe_sym_t;

// Dynamische Tabelle
typedef struct {
    pe_sym_t *items;
    size_t    count;
    size_t    cap;
} pe_symtab_t;

#endif  // __PE_LOADER_H__
