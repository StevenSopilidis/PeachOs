#ifndef _FILE_H_
#define _FILE_H_

#include "pparser.h"


typedef unsigned int FILE_SEEK_MODE;

enum {
    SEEK_SET,
    SEEK_CURRENT,
    SEEK_END
};

typedef unsigned int FILE_MODE;
enum {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};

struct disk;

// function pointer implemented by file systems
// implementation in order to open a file
typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
// function pointer pointer implemented by file system
// implementation and checks and returnsif the disk is formatted
// based on the file system its been implemented by
// returns 0 if the provided disk is using its filesystem
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);

// represents a filesystem
struct  filesystem {
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;  

    char name[20];
};

struct file_descriptor {
    // descriptor index
    int index;
    struct filesystem* filesystem;

    // private data for internal file descriptor
    // will be used by the filesystem
    void* private;

    // The disk that the file descriptor should be used on
    struct disk* disk;
};

// to initialize the filesystem
void fs_init();
// to open a file
int fopen(const char* filename, const char* mode);
// for inserting a filesystem
void fs_insert_filesystem(struct filesystem* filesystem);

struct filesystem* fs_resolve(struct disk* disk);

#endif