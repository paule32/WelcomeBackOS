// iso9660.h - Simple ISO9660 reader for WelcomeBackOS
// (c) 2025 by Jens Kallup - paule32

#ifndef __ISO9660_H__
#define __ISO9660_H__
# pragma once

# include "stdint.h"

# define ISO_LE16(x) ((uint16_t)(x))
# define ISO_LE32(x) ((uint32_t)(x))

// Funktionspointer zum Lesen von Sektoren (LBA-basiert)
typedef void (*iso_read_sectors_t)(uint32_t lba, uint32_t count, void* buffer);

// Unsere "FILE"-Struktur für ISO-Dateien
typedef struct iso_file {
    uint32_t lba;        // Start-LBA der Datei
    uint32_t size;       // Gesamtgröße in Bytes
    uint32_t pos;        // aktuelle Position innerhalb der Datei
} ISO_FILE;

// Optional: Alias, falls du FILE* magst und noch keine libc hast:
typedef ISO_FILE FILE;

// Initialisiert die ISO-Schicht mit einer Funktion, die Sektoren liest.

// Liest den Primary Volume Descriptor, Root-Dir etc.
// Muss nach iso_init() genau einmal aufgerufen werden.
#ifdef __cplusplus
extern "C" int iso_init(iso_read_sectors_t reader);
extern "C" int iso_mount(void);
#else
int iso_init(iso_read_sectors_t reader);
int iso_mount(void);
#endif

// Sucht eine Datei über absoluten Pfad, z.B. "/SHELL.EXE" oder "/DIR/FILE.BIN"
// Gibt 0 bei Erfolg zurück und setzt LBA + Größe.
int iso_find_file(const char* path, uint32_t* out_lba, uint32_t* out_size);

// Öffnet eine Datei als ISO_FILE / FILE*
// path wie bei iso_find_file
#ifdef __cplusplus
extern "C" FILE* file_open(const char* path);
#else
           FILE* file_open(const char* path);
#endif

// Liest bis zu "len" Bytes aus der Datei in "buf".
// Rückgabewert: Anzahl gelesener Bytes (0 = EOF)
#ifdef __cplusplus
extern "C" uint32_t file_read(FILE* f, void* buf, uint32_t len);
#else
           uint32_t file_read(FILE* f, void* buf, uint32_t len);
#endif

// Liest ein einzelnes Byte (wie getch):
// Rückgabewert: 0..255 oder 0xFFFFFFFF bei EOF/Fehler
#ifdef __cplusplus
extern "C" uint32_t file_getch(FILE* f);
#else
           uint32_t file_getch(FILE* f);
#endif

// Setzt Dateiposition absolut (0 = Anfang).
// Gibt 0 bei Erfolg, -1 bei ungültiger Position zurück.
#ifdef __cplusplus
extern "C" int file_seek(FILE* f, uint32_t new_pos);
#else
           int file_seek(FILE* f, uint32_t new_pos);
#endif

// Schließt die Datei (gibt die Struktur frei)
#ifdef __cplusplus
extern "C" void file_close(FILE* f);
#else
           void file_close(FILE* f);
#endif
 
#endif  // __ISO9660_H__
 