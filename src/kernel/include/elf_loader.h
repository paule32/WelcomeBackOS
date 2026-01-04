#ifndef __ELF_LOADER_H__
#define __ELF_LOADER_H__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// ------------------------------------------------------------
// ELF32 Minimalstrukturen
// ------------------------------------------------------------
# define EI_NIDENT 16

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} Elf32_Phdr;

// ------------------------------------------------------------
// Ergebnisinfo für den Caller
// ------------------------------------------------------------
typedef struct {
    uint32_t entry;      // User-EIP
    uint32_t load_bias;  // 0 für ET_EXEC, sonst base-bias
} elf_user_image_t;

# define ELFMAG0 0x7f
# define ELFMAG1 'E'
# define ELFMAG2 'L'
# define ELFMAG3 'F'

# define ELFCLASS32 1
# define ELFDATA2LSB 1

# define ET_EXEC 2
# define ET_DYN  3
# define EM_386  3

# define PT_LOAD 1

# define PF_X 1
# define PF_W 2
# define PF_R 4

# define KEXPORT(sym) \
  __attribute__((section(".ksymtab"), used)) \
  static const ksym_t __ksym_##sym = { #sym, (uint32_t)&sym };

enum {
    SYS_exit  = 1,
    SYS_write = 2
};

typedef struct {
    const char* name;
    uint32_t    addr;
} ksym_t;

extern const ksym_t __ksymtab_start[];
extern const ksym_t __ksymtab_end[];

uint32_t ksym_resolve(const char* name);

#ifdef __cplusplus
};
#endif  // __cplusplus
#endif  // __ELF_LOADER_H__
