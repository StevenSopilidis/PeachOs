#ifndef _DISK_H_
#define _DISK_H_

#include "../fs/file.h"

// things like partitions, real disks, virtaul file systems
typedef unsigned int PEACHOS_DISK_TYPE;

// represents real physical hard disk
#define PEACHOS_DISK_TYPE_REAL 0

struct disk 
{
    PEACHOS_DISK_TYPE type;
    unsigned int sector_size;
    
    // filesystem binded to disk
    struct filesystem* filesystem;
};

void disk_search_and_init();
struct disk* disk_get(int index);
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void *buffer);

#endif