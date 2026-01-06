// pe_loader.c
// Very simple PE32 image loader...
// (c) 2025, 2026 by Jens Kallup - paule32

# include "stdint.h"
# include "proto.h"
# include "kheap.h"

# include "iso9660.h"
# include "pe_loader.h"

// ---------- Helpers ----------
static inline void pe_symtab_init(pe_symtab_t *t) {
    t->items = NULL;
    t->count = 0;
    t->cap   = 0;
}

static inline void pe_symtab_free(pe_symtab_t *t) {
    if (!t) return;
    for (size_t i = 0; i < t->count; i++) {
        free(t->items[i].name);
        t->items[i].name = NULL;
    }
    kfree(t->items);
    t->items = NULL;
    t->count = 0;
    t->cap   = 0;
}

static inline char *pe_strdup(const char *s) {
    if (!s) return NULL;
    size_t n = kstrlen(s) + 1;
    char *p = (char*)kmalloc(n);
    if (!p) return NULL;
    kmemcpy(p, s, n);
    return p;
}

static inline bool pe_symtab_reserve(
    pe_symtab_t *t,
    size_t new_cap) {
        
    if (new_cap <= t->cap) return true;
    pe_sym_t *nitems = (pe_sym_t*)krealloc(t->items, new_cap * sizeof(pe_sym_t));
    if (!nitems) return false;
    t->items = nitems;
    t->cap   = new_cap;
    return true;
}

// Fügt (name, offset) hinzu. Wenn name schon existiert -> aktualisiert offset.
static inline bool pe_symtab_put(
    pe_symtab_t *t,
    const char *name,
    uint32_t offset) {
        
    if (!t || !name) return false;

    // exists? update
    for (size_t i = 0; i < t->count; i++) {
        if (t->items[i].name && kstrcmp(t->items[i].name, name) == 0) {
            t->items[i].offset = offset;
            return true;
        }
    }

    // grow
    if (t->count == t->cap) {
        size_t new_cap = (t->cap == 0) ? 8 : (t->cap * 2);
        if (!pe_symtab_reserve(t, new_cap)) return false;
    }

    // insert
    char *cpy = pe_strdup(name);
    if (!cpy) return false;

    t->items[t->count].name   = cpy;
    t->items[t->count].offset = offset;
    t->count++;
    return true;
}

// Sucht name -> liefert true + schreibt offset
static inline bool pe_symtab_get(
    const pe_symtab_t *t,
    const char *name,
    uint32_t *out_offset) {
        
    if (!t || !name) return false;
    for (size_t i = 0; i < t->count; i++) {
        if (t->items[i].name && kstrcmp(t->items[i].name, name) == 0) {
            if (out_offset) *out_offset = t->items[i].offset;
            return true;
        }
    }
    return false;
}

bool pe32_load(FILE *f, pe_user_image_t *out)
{
    IMAGE_DOS_HEADER dos;
    file_seek(f, 0);
    if (file_read(f, &dos, sizeof(dos)) != sizeof(dos))
        return false;

    if (dos.e_magic != IMAGE_DOS_SIGNATURE)
        return false;

    IMAGE_NT_HEADERS32 nt;
    file_seek(f, dos.e_lfanew);
    if (file_read(f, &nt, sizeof(nt)) != sizeof(nt))
        return false;

    if (nt.Signature != IMAGE_NT_SIGNATURE)
        return false;

    if (nt.OptionalHeader.Magic != 0x10B) // PE32
        return false;

    uint32_t image_base = nt.OptionalHeader.ImageBase;
    uint32_t image_size = nt.OptionalHeader.SizeOfImage;

    void *base = kmalloc(image_size);
    if (!base)
        return false;

    // Sections laden
    IMAGE_SECTION_HEADER sec;
    uint32_t sec_off = dos.e_lfanew +
        sizeof(uint32_t) +
        sizeof(IMAGE_FILE_HEADER) +
        nt.FileHeader.SizeOfOptionalHeader;

    for (int i = 0; i < nt.FileHeader.NumberOfSections; i++) {
        file_seek(f, sec_off + i * sizeof(sec));
        file_read(f, &sec, sizeof(sec));

        void *dst = (uint8_t*)base + sec.VirtualAddress;
        file_seek(f, sec.PointerToRawData);
        file_read(f, dst, sec.SizeOfRawData);
    }

    out->image_base = (uint32_t)base;
    out->image_size = image_size;
    out->entry      = (uint32_t)base + nt.OptionalHeader.AddressOfEntryPoint;

    return true;
}

typedef void (*pe_entry_t)(void);
void pe32_start_user(const pe_user_image_t *img) {
    ((pe_entry_t)img->entry)();
}
void test_app(void)
{
    pe_user_image_t img;
    FILE *f = file_open("shell32.exe");
    if (!f) {
        printformat("shell32.exe could not open.\n");
        return;
    }
    if (pe32_load(f, &img)) {
        pe32_start_user(&img);
        file_close(f);
        return;
    }
}
