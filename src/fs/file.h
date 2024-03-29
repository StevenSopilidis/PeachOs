#ifndef _FILE_H_
#define _FILE_H_

#include "pparser.h"
#include "stdint.h"

#define FS_DIRECTORY 0
#define FS_FILE 1


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
// function for read from file
// @disk: disk to read from
// @private: fs private data
// @size: size of data to read
// @nmemb: number of blocks of size @size to read 
// for e.g if size=10 and nmemb=5 wil will read
// 50 bytes
// @out: buffer to put the data
typedef int (*FS_READ_FUNCTION)(struct disk* disk,void* private, uint32_t size, uint32_t nmemb, char* out);

// function pointer for closing a file
typedef int (*FS_CLOSE_FUNCTION)(void* private);

// function pointer for seek function of filesystem
typedef int (*FS_SEEK_FUNCTION)(void* private, uint32_t offset, FILE_SEEK_MODE seek_mode);

// function for creating an fs item (file or directory)
typedef int (*FS_CREATE_ITEM_FUNCTION) (struct disk* disk, char* name, char* ext, int type, struct path_part* path);

// function for writing to a file
typedef int (*FS_WRITE_FUNCTION)(struct disk* disk, void* private, void* data, uint32_t size, uint32_t offset);

enum 
{
    FILE_STAT_READ_ONLY = 0b00000001
};

typedef unsigned int FILE_STAT_FLAGS;


struct file_stat {
    FILE_STAT_FLAGS flags;
    uint32_t file_size;
};

// function pointer for fstat function of filesystem
typedef int (*FS_STAT_FUNCTION)(struct disk* disk, void* private, struct file_stat* stat);

// represents a filesystem
struct  filesystem {
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;  
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    FS_STAT_FUNCTION stat;
    FS_CLOSE_FUNCTION close;
    FS_CREATE_ITEM_FUNCTION create;
    FS_WRITE_FUNCTION write;

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
int fopen(const char* filename, const char* mode_string);
// to read file
int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd);
// to write to a file
int fwrite(void* data, uint32_t size, uint32_t offset, int fd);
// to seek a file
int fseek(int fd, uint32_t offset, FILE_SEEK_MODE whence);
// for inserting a filesystem
void fs_insert_filesystem(struct filesystem* filesystem);
// for getting stats on file
int fstat(int fd, struct file_stat* stat);
// for closing a file
int fclose(int fd);
// for creating an item
// @path: path to create item to
// @type: 0 for file, 1 for directory  
int fcreate(char* name, char* ext, int type, const char* path);

struct filesystem* fs_resolve(struct disk* disk);

#endif