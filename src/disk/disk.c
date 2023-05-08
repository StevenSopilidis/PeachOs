#include "../io/io.h"
#include "../status.h"
#include "disk.h"
#include "../memory/memory.h"
#include "../config.h"
#include "../status.h"

struct disk disk;

// method for reading sectors of disk
// @lba: address of block 
// @total: total numbers of sectors to read
// @buffer: buffer that will hold the data
int disk_read_sector(int lba, int total, void* buffer) {
    outb(0x1F6, (lba >> 24) | 0xE0);
    outb(0x1F2, total);
    outb(0x1F3, (unsigned char)(lba & 0xff));
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20);

    unsigned short* ptr = (unsigned short*) buffer;
    for (int b = 0; b < total; b++)
    {
        // Wait for the buffer to be ready
        char c = insb(0x1F7);
        while(!(c & 0x08))
        {
            c = insb(0x1F7);
        }

        // Copy from hard disk to memory
        for (int i = 0; i < 256; i++)
        {
            *ptr = insw(0x1F0);
            ptr++;
        }

    }
    return PEACHOS_ALL_OK;
}

// func that will search for disks and initializing them
// for now it will just point to primary disk and initialize it
void disk_search_and_init() {
    memset(&disk, 0, sizeof(disk));
    disk.type = PEACHOS_DISK_TYPE_REAL;
    disk.sector_size = PEACHOS_SECTOR_SIZE;
    disk.filesystem = fs_resolve(&disk);
}

// func for getting a disk base on index
// for now we just return the main disk
// so we could have an array of the different disks 
// that the system would use and pull from it
struct disk* disk_get(int index) {
    if (index !=0 )
        return 0;
    return &disk;
}

// func for reading from specified disk
// for now just reads from main disks
int disk_read_block(struct disk* idisk, unsigned int lba, int total, void *buffer) {
    if (idisk != &disk) {
        return -EIO;
    }
    return disk_read_sector(lba, total, buffer);
}