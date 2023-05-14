#include "file.h"
#include "../config.h"
#include "../kernel.h"
#include "../memory/memory.h"
#include "../memory/heap/kheap.h"
#include "../status.h"
#include "fat/fat16.h"

// represents every filesystem in our os
struct filesystem* filesystems[PEACHOS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[PEACHOS_MAX_FILE_DESCRIPTORS];

// gets a free slot from filesystems
// returns double pointer so we can later change the values
static struct filesystem** fs_get_free_file_system() {
    for (int i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] == 0)
            return &filesystems[i];
    }
    return 0;
}

// will be used by drivers to insert their filesystems
// into the os
void fs_insert_filesystem(struct filesystem* filesystem) {
    struct filesystem** fs = fs_get_free_file_system();
    if (!fs) {
        // TODO: panic()
        print("Problem inserting filesystem");
        while(1) {}
    }
    *fs = filesystem;
}

// will load static file systems,
// file systems that are built into the core kernel
// in our case that is fat16 filesystem
static void fs_static_load() {
    fs_insert_filesystem(fat16_init());
}

// will load all available filesystems
void fs_load() {
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

// initializes filesystem
void fs_init() {
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

// function for creating a file descriptor
static int file_new_descriptor(struct file_descriptor** desc_out) {
    int res = -ENOMEM;
    for (int i = 0; i < PEACHOS_MAX_FILE_DESCRIPTORS; i++)
    {
        if (file_descriptors[i] == 0) {
            struct file_descriptor*  desc = kzalloc(sizeof(struct file_descriptor));
            // descriptors start at 1
            desc->index = i + 1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }

    return res;
}

// function that gets filedescriptor based on id
static struct file_descriptor* file_get_descriptor(int fd) {
    if(fd <=0 || fd >= PEACHOS_MAX_FILE_DESCRIPTORS)
        return 0;
    // descriptors start at 1
    int index = fd - 1;
    return file_descriptors[index];
}


// function for resolving the filesystem of our os
struct filesystem* fs_resolve(struct disk* disk) {
    struct filesystem* fs = 0;  
    for (int i = 0; i < PEACHOS_MAX_FILESYSTEMS; i++)
    {
        if(filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0) {
            fs = filesystems[i];
            break;
        }
    }
    return fs;
}

int fopen(const char* filename, const char* mode) {
    return -EIO;
}