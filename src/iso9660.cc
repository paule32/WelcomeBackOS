// iso9660.c - Simple ISO9660 reader for WelcomeBackOS
// (c) 2025 by Jens Kallup - paule32

# include "stdint.h"
# include "kheap.h"   // für kmalloc/kfree
# include "iso9660.h"
# include "proto.h"

# define DESKTOP
# include "vga.h"

extern "C" void enter_usermode(void);
extern void enter_shell(void);

// ----------------------------------------
// interne Helper
// ----------------------------------------

// Primary Volume Descriptor (nur relevante Teile)
typedef struct __attribute__((packed)) {
    uint8_t  type;          // 1 = Primary Volume Descriptor
    char     id[5];         // "CD001"
    uint8_t  version;
    uint8_t  unused1;
    char     system_id[32];
    char     volume_id[32];
    uint8_t  unused2[8];
    uint32_t volume_space_le;
    uint32_t volume_space_be;
    uint8_t  unused3[32];
    uint16_t volume_set_size_le;
    uint16_t volume_set_size_be;
    uint16_t volume_seq_le;
    uint16_t volume_seq_be;
    uint16_t logical_block_size_le;
    uint16_t logical_block_size_be;
    uint32_t path_table_size_le;
    uint32_t path_table_size_be;
    uint32_t type_l_path_table_loc;
    uint32_t opt_type_l_path_table_loc;
    uint32_t type_m_path_table_loc;
    uint32_t opt_type_m_path_table_loc;
    uint8_t  root_dir_record[34];  // Directory Record für Root
    // ... Rest ignorieren wir
} iso_pvd_t;

// Directory Record (vereinfachte Sicht)
typedef struct __attribute__((packed)) {
    uint8_t  length;           // Länge dieses Records
    uint8_t  ext_attr_length;
    uint32_t extent_le;        // LBA (little endian)
    uint32_t extent_be;
    uint32_t size_le;          // Dateigröße in Bytes
    uint32_t size_be;
    uint8_t  date[7];
    uint8_t  flags;            // Bit 1 = Directory
    uint8_t  file_unit_size;
    uint8_t  interleave;
    uint16_t vol_seq_le;
    uint16_t vol_seq_be;
    uint8_t  name_len;
    char     name[];           // direkt anschließend, nicht 0-terminiert
} iso_dir_record_t;

// ----------------------------------------
// globale ISO-Infos
// ----------------------------------------

static iso_read_sectors_t iso_read_sectors = 0;
static uint32_t iso_block_size  = 2048;  // default
static uint32_t iso_root_lba    = 0;
static uint32_t iso_root_size   = 0;
static int      iso_is_mounted  = 0;

static uint32_t align_up(uint32_t value, uint32_t align)
{
    uint32_t mask = align - 1;
    return (value + mask) & ~mask;
}

static char to_upper(char c)
{
    if (c >= 'a' && c <= 'z')
        return c - 32;
    return c;
}

// ----------------------------------------
// init / mount
// ----------------------------------------
int iso_init(iso_read_sectors_t reader)
{
    if (!reader) {
        gfx_printf("ISO9660: reader error.\n");
        return -1;
    }
    iso_read_sectors = reader;
    iso_is_mounted = 0;
    
    if (iso_mount() != 0) {
        gfx_printf("ISO mount Error.\n");
        return -1;
    }   else {
        gfx_printf("ISO mount successfully.\n");
        enter_usermode();
    }
    return 0;
}

extern "C" int iso_mount(void)
{
    if (!iso_read_sectors)
    return -1;

    // PVD liegt bei LBA 16
    uint8_t* buf = (uint8_t*)kmalloc(2048);
    if (!buf) return -2;

    iso_read_sectors(16, 1, buf);

    iso_pvd_t* pvd = (iso_pvd_t*)buf;
    if (pvd->type != 1 ||
        pvd->id[0] != 'C' ||
        pvd->id[1] != 'D' ||
        pvd->id[2] != '0' ||
        pvd->id[3] != '0' ||
        pvd->id[4] != '1') {
        kfree(buf);
        return -3;
    }

    // logische Blockgröße
    uint16_t lbsize = pvd->logical_block_size_le;
    if (lbsize == 0) lbsize = 2048;
    iso_block_size = lbsize;

    // Root Directory Record auslesen
    iso_dir_record_t* root = (iso_dir_record_t*)pvd->root_dir_record;
    iso_root_lba  = root->extent_le;
    iso_root_size = root->size_le;

    iso_is_mounted = 1;

    kfree(buf);
    return 0;
}

// ----------------------------------------
// Directory-Scans
// ----------------------------------------

// Vergleich von Name im Directory (ISO-Format) mit einem "normalen" Pfad-Element
// - ISO-Name ist nicht 0-terminiert, enthält oft ";1" am Ende
// - wir vergleichen case-insensitive und ignorieren ";version"
static int iso_name_equals(const char* iso_name, uint8_t iso_len, const char* wanted)
{
    // 1. ISO-Name in Temp-Puffer kopieren, ;Version abschneiden
    char temp[64];
    if (iso_len >= sizeof(temp)) iso_len = sizeof(temp) - 1;

    uint8_t j = 0;
    for (; j < iso_len; ++j) {
        if (iso_name[j] == ';') break; // kein Versions-Suffix vergleichen
        temp[j] = to_upper(iso_name[j]);
    }
    temp[j] = 0;

    // 2. wanted in uppercase vergleichen
    char wbuf[64];
    uint32_t k = 0;
    while (wanted[k] && k < sizeof(wbuf)-1) {
        wbuf[k] = to_upper(wanted[k]);
        ++k;
    }
    wbuf[k] = 0;
    int ret = kstrcmp(temp, wbuf);
    return ret;
}

// Sucht ein Element (Datei oder Verzeichnis) in einem Directory-Block
// dir_data/dir_size = Daten des Verzeichnisses
// wanted = Name ohne Slash
// out_lba/out_size: LBA+Size des Eintrags
// is_dir: 1 wenn Verzeichnis, 0 wenn Datei
static int iso_find_in_dir(uint8_t* dir_data, uint32_t dir_size,
                           const char* wanted,
                           uint32_t* out_lba, uint32_t* out_size,
                           int* is_dir_out)
{
    uint32_t offset = 0;

    while (offset < dir_size) {
        iso_dir_record_t* rec = (iso_dir_record_t*)(dir_data + offset);
        if (rec->length == 0) {
            // Padding bis Blockende überspringen
            uint32_t next_block = align_up(offset + 1, iso_block_size);
            if (next_block <= offset) break;
            offset = next_block;
            continue;
        }

        uint8_t name_len = rec->name_len;
        char*   name     = rec->name;

        // "." und ".." überspringen (0x00, 0x01)
        if (!(name_len == 1 && (name[0] == 0 || name[0] == 1))) {
            if (iso_name_equals(name, name_len, wanted) == 0) {
                if (out_lba)  *out_lba  = rec->extent_le;
                if (out_size) *out_size = rec->size_le;
                if (is_dir_out)
                    *is_dir_out = (rec->flags & 0x02) ? 1 : 0;
                return 0;
            }
        }

        offset += rec->length;
    }

    return -1; // nicht gefunden
}

// Zerteile Pfad wie "/DIR/FILE.BIN" in Komponenten und laufe Verzeichnisse ab
int iso_find_file(const char* path, uint32_t* out_lba, uint32_t* out_size)
{
    if (!iso_is_mounted || !path || !out_lba || !out_size)
        return -1;

    // Start: Root Directory
    uint32_t cur_lba  = iso_root_lba;
    uint32_t cur_size = iso_root_size;

    // Pfad-Parser
    const char* p = path;

    // führende Slashes skippen
    while (*p == '/') ++p;
    if (*p == 0) {
        // Pfad ist "/" -> Root-Verzeichnis selbst
        *out_lba  = cur_lba;
        *out_size = cur_size;
        return 0;
    }

    char component[64];

    while (*p) {
        // Komponente extrahieren (bis '/' oder Ende)
        uint32_t clen = 0;
        while (*p && *p != '/' && clen < sizeof(component)-1) {
            component[clen++] = *p++;
        }
        component[clen] = 0;

        // evtl. weiteren Slash überspringen
        while (*p == '/') ++p;

        // Aktuelles Verzeichnis lesen
        uint32_t blocks = align_up(cur_size, iso_block_size) / iso_block_size;
        uint8_t* dirbuf = (uint8_t*)kmalloc(blocks * iso_block_size);
        if (!dirbuf) return -2;

        iso_read_sectors(cur_lba, blocks, dirbuf);

        int is_dir = 0;
        uint32_t next_lba = 0;
        uint32_t next_size = 0;

        int res = iso_find_in_dir(dirbuf, cur_size, component,
                                  &next_lba, &next_size, &is_dir);

        kfree(dirbuf);

        if (res != 0) {
            return -3; // Komponente nicht gefunden
        }

        // Falls noch weitere Komponenten folgen, MUSS das ein Directory sein
        if (*p != 0 && !is_dir) {
            return -4; // Datei statt Verzeichnis im Weg
        }

        cur_lba  = next_lba;
        cur_size = next_size;
    }

    // Letzte Komponente war die Datei
    *out_lba  = cur_lba;
    *out_size = cur_size;
    return 0;
}

// ----------------------------------------
// FILE-API
// ----------------------------------------

FILE* file_open(const char* path)
{
    uint32_t lba, size;
    if (iso_find_file(path, &lba, &size) != 0)
        return 0;

    ISO_FILE* f = (ISO_FILE*)kmalloc(sizeof(ISO_FILE));
    if (!f) return 0;

    f->lba  = lba;
    f->size = size;
    f->pos  = 0;
    return (FILE*)f;
}

uint32_t file_read(FILE* file, void* buf, uint32_t len)
{
    if (!file || !buf || len == 0 || !iso_read_sectors)
        return 0;

    ISO_FILE* f = (ISO_FILE*)file;
    
    if (f->pos >= f->size)
        return 0;

    if (f->pos + len > f->size)
        len = f->size - f->pos;

    uint8_t* out = (uint8_t*)buf;

    // Blockweises Lesen
    uint32_t remaining = len;
    while (remaining > 0) {
        uint32_t block_offset = f->pos % iso_block_size;
        uint32_t block_lba    = f->lba + (f->pos / iso_block_size);

        uint8_t sector_buf[2048];  // Annahme: Blockgröße <= 2048
        uint32_t block_read_size = iso_block_size;
        if (block_read_size > sizeof(sector_buf))
            block_read_size = sizeof(sector_buf);

        iso_read_sectors(block_lba, 1, sector_buf);

        uint32_t chunk = iso_block_size - block_offset;
        if (chunk > remaining)
            chunk = remaining;

        for (uint32_t i = 0; i < chunk; ++i)
            out[i] = sector_buf[block_offset + i];

        out      += chunk;
        f->pos   += chunk;
        remaining -= chunk;
    }

    return len;
}

uint32_t file_getch(FILE* file)
{
    uint8_t c;
    uint32_t n = file_read(file, &c, 1);
    if (n == 0)
        return 0xFFFFFFFFU;
    return (uint32_t)c;
}

int file_seek(FILE* file, uint32_t new_pos)
{
    if (!file) return -1;
    ISO_FILE* f = (ISO_FILE*)file;
    
    if (new_pos > f->size) return -1;
    f->pos = new_pos;
    
    return 0;
}

void file_close(FILE* file)
{
    if (!file) return;
    ISO_FILE* f = (ISO_FILE*)file;
    kfree(f);
}
