#ifndef INITRD_H
#define INITRD_H

#include "os.h"
#include "fs.h"

typedef struct
{
    ULONG nfiles; // The number of files in the ramdisk.
} initrd_header_t;

typedef struct
{
    ULONG magic;    // Magic number, for error checking.
    char  name[64]; // Filename.
    ULONG off;      // Offset in the initrd that the file starts.
    ULONG length;   // Length of the file.
} initrd_file_header_t;

// Installs the initial ramdisk. It gets passed the address, and returns a completed filesystem node.
extern fs_node_t* install_initrd(ULONG location);

#endif
