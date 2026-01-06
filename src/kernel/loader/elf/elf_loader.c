// ---------------------------------------------------------------------------
// \file  ELF_loader.c
// \note  (c) 2025 by Jens Kallup - paule32
//        all rights reserved.
// ---------------------------------------------------------------------------
# include "config.h"

# include "stdint.h"
# include "kheap.h"
# include "iso9660.h"
# include "elf_loader.h"

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------
static bool read_exact(FILE* f, uint32_t off, void* dst, uint32_t len) {
    // passe das an deinen file_read an:
    // z.B. file_read(f, off, len, dst) -> return == len
    file_seek(f, off);
    uint32_t l = file_read(f, (uint8_t*)dst, len);
    if (l == len) {
        return true;
    }   return false;
}
static inline uint32_t align_down(uint32_t x, uint32_t a) { return x & ~(a - 1u); }
static inline uint32_t align_up  (uint32_t x, uint32_t a) { return (x + a - 1u) & ~(a - 1u); }

// Optional: sehr einfache Range-Checks (an dein Layout anpassen)
static bool user_range_ok(uint32_t start, uint32_t size) {
    uint32_t end = start + size;
    // Beispiel: User ab 1MB bis vor Kernel-Split 0xC0000000
    if (start < 0x00100000) return false;
    if (end   >= 0xC0000000) return false;
    if (end < start) return false; // overflow
    return true;
}

// ------------------------------------------------------------
// Hauptfunktion
// base_for_dyn: gewünschte Ladebasis für ET_DYN, z.B. 0x00400000
// Bei ET_EXEC wird base_for_dyn ignoriert.
// ------------------------------------------------------------
bool elf32_load_nomap(FILE* f, uint32_t base_for_dyn, elf_user_image_t* out) {
    if (!f || !out) return false;

    Elf32_Ehdr eh;
    if (!read_exact(f, 0, &eh, sizeof(eh))) return false;

    // Magic + Class/Data
    if (eh.e_ident[0] != ELFMAG0 || eh.e_ident[1] != ELFMAG1 ||
        eh.e_ident[2] != ELFMAG2 || eh.e_ident[3] != ELFMAG3) return false;
    if (eh.e_ident[4] != ELFCLASS32) return false;
    if (eh.e_ident[5] != ELFDATA2LSB) return false;

    if (eh.e_machine != EM_386) return false;
    if (!(eh.e_type == ET_EXEC || eh.e_type == ET_DYN)) return false;

    if (eh.e_phentsize != sizeof(Elf32_Phdr)) return false;
    if (eh.e_phnum == 0 || eh.e_phnum > 128) return false;

    // Program Header Tabelle einlesen (kleiner Kernel-Stack? -> besser static/heap)
    //Elf32_Phdr ph[128];
    Elf32_Phdr* ph = kmalloc(eh.e_phnum * sizeof(Elf32_Phdr));
    if (!ph) {
        printformat("elf32 ph error.\n");
        return false;
    }
    uint32_t ph_bytes = eh.e_phnum * sizeof(Elf32_Phdr);
    if (!read_exact(f, eh.e_phoff, ph, ph_bytes)) return false;

    // Load bias berechnen (nur ET_DYN)
    uint32_t bias = 0;
    if (eh.e_type == ET_DYN) {
        // Wir setzen: runtime_addr = base_for_dyn + (p_vaddr - min_vaddr_page)
        uint32_t min_v = 0xFFFFFFFFu;
        for (uint32_t i = 0; i < eh.e_phnum; i++) {
            if (ph[i].p_type != PT_LOAD) continue;
            if (ph[i].p_memsz == 0) continue;
            if (ph[i].p_vaddr < min_v) min_v = ph[i].p_vaddr;
        }
        if (min_v == 0xFFFFFFFFu) return false;

        // min_vaddr auf Page runterziehen (robuster)
        uint32_t min_page = align_down(min_v, 4096);
        bias = base_for_dyn - min_page;
    }

    // Segmente laden
    for (uint32_t i = 0; i < eh.e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD) continue;
        if (ph[i].p_memsz == 0) continue;
        if (ph[i].p_filesz > ph[i].p_memsz) return false;

        uint32_t dst_va = ph[i].p_vaddr + bias;
        uint32_t memsz  = ph[i].p_memsz;
        uint32_t filesz = ph[i].p_filesz;

        // Simple user-range guard
        if (!user_range_ok(dst_va, memsz)) return false;

        // Dateiinhalt -> Zieladresse
        if (filesz) {
            if (!read_exact(f, ph[i].p_offset, (void*)dst_va, filesz)) return false;
        }

        // BSS -> 0
        if (memsz > filesz) {
            kmemset((void*)(dst_va + filesz), 0, memsz - filesz);
        }
    }

    out->load_bias = bias;
    out->entry     = eh.e_entry + bias;
    return true;
}
