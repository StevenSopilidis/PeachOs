#ifndef _DISK_H_
#define _DISK_H_

#include "../fs/file.h"
#include "stdint.h"

// things like partitions, real disks, virtaul file systems
typedef unsigned int PEACHOS_DISK_TYPE;

// represents real physical hard disk
#define PEACHOS_DISK_TYPE_REAL 0

struct disk 
{
    //id of the disk
    int id;

    PEACHOS_DISK_TYPE type;
    unsigned int sector_size;
    
    // filesystem binded to disk
    struct filesystem* filesystem;

    // private data of our filesystem
    void* fs_private;
};

extern void lba_write(unsigned int lba, int total, uint32_t addr);

void disk_search_and_init();
struct disk* disk_get(int index);
// function for reading sectors
// from specified disk
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void *buffer);
// function for writinng a sector
// to specified disk
int disk_write_block(struct disk* idisk, unsigned int lba, int total, uint32_t addr);



#endif