#ifndef FS_H
#define FS_H

#include "os.h"

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08 // Is the file an active mountpoint?

struct dirent;
struct fs_node;

// These typedefs define the type of callbacks - called when read/write/open/close are called.
typedef ULONG           (*read_type_t)    (struct fs_node*,ULONG,ULONG,unsigned char*);
typedef ULONG           (*write_type_t)   (struct fs_node*,ULONG,ULONG,unsigned char*);
typedef void            (*open_type_t)    (struct fs_node*);
typedef void            (*close_type_t)   (struct fs_node*);
typedef struct dirent*  (*readdir_type_t) (struct fs_node*,ULONG);
typedef struct fs_node* (*finddir_type_t) (struct fs_node*,char* name);

typedef struct fs_node
{
    char  name[128];// The filename.
    ULONG mask;     // The permissions mask.
    ULONG uid;      // The owning user.
    ULONG gid;      // The owning group.
    ULONG flags;    // Includes the node type. See #defines above.
    ULONG inode;    // This is device-specific - provides a way for a filesystem to identify files.
    ULONG length;   // Size of the file, in bytes.
    ULONG impl;     // An implementation-defined number.
    read_type_t     read;
    write_type_t    write;
    open_type_t     open;
    close_type_t    close;
    readdir_type_t  readdir;
    finddir_type_t  finddir;
    struct fs_node* ptr; // Used by mountpoints and symlinks.
} fs_node_t;

struct dirent
{
    char name[128]; // Filename.
    ULONG ino;      // Inode number. Required by POSIX.
};

extern fs_node_t* fs_root; // The root of the filesystem.

// Standard read/write/open/close functions. Note that these are all suffixed with _fs to distinguish them
// from the read/write/open/close which deal with file descriptors, not file nodes.
ULONG          read_fs    (fs_node_t* node, ULONG offset, ULONG size, unsigned char* buffer);
ULONG          write_fs   (fs_node_t* node, ULONG offset, ULONG size, unsigned char* buffer);
void           open_fs    (fs_node_t* node, unsigned char read,   unsigned char write);
void           close_fs   (fs_node_t* node);
struct dirent* readdir_fs (fs_node_t* node, ULONG index);
fs_node_t*     finddir_fs (fs_node_t* node, char* name);

#endif
