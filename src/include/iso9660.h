// iso9660.h - Simple ISO9660 reader for WelcomeBackOS
// (c) 2025 by Jens Kallup - paule32

#ifndef __ISO9660_H__
#define __ISO9660_H__
# pragma once

# include "stdint.h"

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
extern "C" void iso_init(iso_read_sectors_t reader);
extern "C" int  iso_mount(void);
#else
void iso_init(iso_read_sectors_t reader);
int iso_mount(void);
#endif

// Sucht eine Datei über absoluten Pfad, z.B. "/SHELL.EXE" oder "/DIR/FILE.BIN"
// Gibt 0 bei Erfolg zurück und setzt LBA + Größe.
int iso_find_file(const char* path, uint32_t* out_lba, uint32_t* out_size);

// Öffnet eine Datei als ISO_FILE / FILE*
// path wie bei iso_find_file
FILE* file_open(const char* path);

// Liest bis zu "len" Bytes aus der Datei in "buf".
// Rückgabewert: Anzahl gelesener Bytes (0 = EOF)
uint32_t file_read(FILE* f, void* buf, uint32_t len);

// Liest ein einzelnes Byte (wie getch):
// Rückgabewert: 0..255 oder 0xFFFFFFFF bei EOF/Fehler
uint32_t file_getch(FILE* f);

// Setzt Dateiposition absolut (0 = Anfang).
// Gibt 0 bei Erfolg, -1 bei ungültiger Position zurück.
int file_seek(FILE* f, uint32_t new_pos);

// Schließt die Datei (gibt die Struktur frei)
void file_close(FILE* f);
 
#endif  // __ISO9660_H__
 