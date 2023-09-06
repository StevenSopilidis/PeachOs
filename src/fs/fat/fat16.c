#include "fat16.h"
#include "string/string.h"
#include "disk/disk.h"
#include "disk/stream.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"
#include "kernel.h"
#include <stdint.h>
#include "config.h"

#define PEACHOS_FAT16_SIGNATURE 0x29
#define PEACHOS_FAT16_FAT_ENTRY_SIZE 0x02
#define PEACHOS_FAT16_BAD_SECTOR 0xFF7
#define PEACHOS_FAT16_UNUSED 0x00


typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// Fat directory entry attributes bitmask
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80


struct fat_header_extended
{
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct fat_header
{
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_setors;
    uint32_t sectors_big;
} __attribute__((packed));

struct fat_h
{
    struct fat_header primary_header;
    union fat_h_e
    {
        struct fat_header_extended extended_header;
    } shared;
};

// struct representing a file or subdirectory
struct fat_directory_item
{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

// struct representing a directory
struct fat_directory
{
    struct fat_directory_item* item;
    int total;
    int sector_pos;
    int ending_sector_pos;
};

// struct representing fat item either a file or directory
struct fat_item
{
    union 
    {
        struct fat_directory_item* item;
        struct fat_directory* directory;
    };
    
    FAT_ITEM_TYPE type;
};

// represents a file descriptor
struct fat_file_descriptor
{
    struct fat_item* item;
    uint32_t pos;
};

// private data of our filesystem
struct fat_private
{
    struct fat_h header;
    struct fat_directory root_directory;

    // Used to stream data clusters
    struct disk_stream* cluster_read_stream;
    // Used to stream the file allocation table
    struct disk_stream* fat_read_stream;

    // Used in situations where we stream the directory
    struct disk_stream* directory_stream;
};

int fat16_resolve(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);
int fat16_read(struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr);
int fat16_write(struct disk* disk, void* descriptor, void* data, uint32_t size, uint32_t offset);
int fat16_seek(void *private, uint32_t offset, FILE_SEEK_MODE seek_mode);
int fat16_stat(struct disk* disk, void* private, struct file_stat* stat);
int fat16_close(void* private);
int fat16_create(struct disk* disk, char* name, char* ext, int type, struct path_part* path);


struct filesystem fat16_fs =
{
    .resolve = fat16_resolve,
    .open = fat16_open,
    .read = fat16_read,
    .write = fat16_write,
    .seek = fat16_seek,
    .stat = fat16_stat,
    .close= fat16_close,
    .create = fat16_create,
};

// initializes the fat16 filesystem
struct filesystem* fat16_init()
{
    strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}

// initialize private data for fat16 filesystem
static void fat16_init_private(struct disk* disk, struct fat_private* private)
{
    memset(private, 0, sizeof(struct fat_private));
    private->cluster_read_stream = diskstream_new(disk->id);
    private->fat_read_stream = diskstream_new(disk->id);
    private->directory_stream = diskstream_new(disk->id);
}

// converts sector number to absolute value in bytes
int fat16_sector_to_absolute(struct disk* disk, int sector)
{
    return sector * disk->sector_size;
}

// function for getting all the items from a directory given its
// starting sector
// returns the number of items in the directory
int fat16_get_total_items_for_directory(struct disk* disk, uint32_t directory_start_sector)
{
    struct fat_directory_item item;
    struct fat_directory_item empty_item;
    memset(&empty_item, 0, sizeof(empty_item));
    
    struct fat_private* fat_private = disk->fs_private;

    int res = 0;
    int i = 0;
    int directory_start_pos = directory_start_sector * disk->sector_size;
    struct disk_stream* stream = fat_private->directory_stream;
    if(disk_seek(stream, directory_start_pos) != PEACHOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    while(1)
    {
        if (diskstream_read(stream, &item, sizeof(item)) != PEACHOS_ALL_OK)
        {
            res = -EIO;
            goto out;
        }

        // the first byte of the filename indicates
        // if we should continue searching or not
        if (item.filename[0] == 0x00)
        {
            // We are done
            break;
        }

        // Is the item unused
        if (item.filename[0] == 0xE5)
        {
            continue;
        }

        i++;
    }

    res = i;

out:
    return res;
}

// function that get the root directory of the fs
// and returns it via directory parameter
int fat16_get_root_directory(struct disk* disk, struct fat_private* fat_private, struct fat_directory* directory)
{
    int res = 0;
    struct fat_header* primary_header = &fat_private->header.primary_header;
    int root_dir_sector_pos = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;
    int root_dir_entries = fat_private->header.primary_header.root_dir_entries;
    int root_dir_size = (root_dir_entries * sizeof(struct fat_directory_item));
    int total_sectors = root_dir_size / disk->sector_size;
    if (root_dir_size % disk->sector_size)
    {
        total_sectors += 1;
    }

    int total_items = fat16_get_total_items_for_directory(disk, root_dir_sector_pos);

    struct fat_directory_item* dir = kzalloc(root_dir_size);
    if (!dir)
    {
        res = -ENOMEM;
        goto out;
    }

    struct disk_stream* stream = fat_private->directory_stream;
    if (disk_seek(stream, fat16_sector_to_absolute(disk, root_dir_sector_pos)) != PEACHOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    if (diskstream_read(stream, dir, root_dir_size) != PEACHOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    directory->item = dir;
    directory->total = total_items;
    directory->sector_pos = root_dir_sector_pos;
    directory->ending_sector_pos = root_dir_sector_pos + (root_dir_size / disk->sector_size);
out:
    return res;
}

// function that resolves wether or not the fat16 filesystem
// can be loaded
int fat16_resolve(struct disk* disk)
{
    int res = 0;
    struct fat_private* fat_private = kzalloc(sizeof(struct fat_private));
    fat16_init_private(disk, fat_private);

    disk->fs_private = fat_private;
    disk->filesystem = &fat16_fs;
    
    struct disk_stream* stream = diskstream_new(disk->id);
    if(!stream)
    {
        res = -ENOMEM;
        goto out;
    }

    if (diskstream_read(stream, &fat_private->header, sizeof(fat_private->header)) != PEACHOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    if (fat_private->header.shared.extended_header.signature != 0x29)
    {
        res = -EFSNOTUS;
        goto out;
    }

    if (fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != PEACHOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

out:
    if (stream)
    {
        diskstream_close(stream);
    }

    if (res < 0)
    {
        kfree(fat_private);
        disk->fs_private = 0;
    }
    return res;
}

void fat16_to_proper_string(char** out, const char* in)
{
    while(*in != 0x00 && *in != 0x20)
    {
        **out = *in;
        *out += 1;
        in +=1;
    }

    if (*in == 0x20)
    {
        **out = 0x00;
    }
}


void fat16_get_full_relative_filename(struct fat_directory_item* item, char* out, int max_len)
{
    memset(out, 0x00, max_len);
    char *out_tmp = out;
    fat16_to_proper_string(&out_tmp, (const char*) item->filename);
    if (item->ext[0] != 0x00 && item->ext[0] != 0x20)
    {
        *out_tmp++ = '.';
        fat16_to_proper_string(&out_tmp, (const char*) item->ext);
    }

}

struct fat_directory_item* fat16_clone_directory_item(struct fat_directory_item* item, int size)
{
    struct fat_directory_item* item_copy = 0;
    if (size < sizeof(struct fat_directory_item))
    {
        return 0;
    }

    item_copy = kzalloc(size);
    if (!item_copy)
    {
        return 0;
    }

    memcpy(item_copy, item, size);
    return item_copy;
}

static uint32_t fat16_get_first_cluster(struct fat_directory_item* item)
{
    return (item->high_16_bits_first_cluster) | item->low_16_bits_first_cluster;
};

static int fat16_cluster_to_sector(struct fat_private* private, int cluster)
{
    return private->root_directory.ending_sector_pos + ((cluster - 2) * private->header.primary_header.sectors_per_cluster);
}

static uint32_t fat16_get_first_fat_sector(struct fat_private* private)
{
    return private->header.primary_header.reserved_sectors;
}

// function that gets the position of the fat table
static uint32_t fat16_get_fat_position(struct disk* disk) {
    return fat16_get_first_fat_sector((struct fat_private*)disk->fs_private) * disk->sector_size;
} 

static int fat16_get_fat_entry(struct disk* disk, int cluster)
{
    int res = -1;
    struct fat_private* private = disk->fs_private;
    struct disk_stream* stream = private->fat_read_stream;
    if (!stream)
    {
        goto out;
    }

    uint32_t fat_table_position = fat16_get_first_fat_sector(private) * disk->sector_size;
    res = disk_seek(stream, fat_table_position * (cluster * PEACHOS_FAT16_FAT_ENTRY_SIZE));
    if (res < 0)
    {
        goto out;
    }

    uint16_t result = 0;
    res = diskstream_read(stream, &result, sizeof(result));
    if (res < 0)
    {
        goto out;
    }

    res = result;
out:
    return res;
}
/**
 * Gets the correct cluster to use based on the starting cluster and the offset
 */
static int fat16_get_cluster_for_offset(struct disk* disk, int starting_cluster, int offset)
{
    int res = 0;
    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = starting_cluster;
    int clusters_ahead = offset / size_of_cluster_bytes;
    for (int i = 0; i < clusters_ahead; i++)
    {
        int entry = fat16_get_fat_entry(disk, cluster_to_use);
        if (entry == 0xFF8 || entry == 0xFFF)
        {
            // We are at the last entry in the file
            res = -EIO;
            goto out;
        }

        // Sector is marked as bad?
        if (entry == PEACHOS_FAT16_BAD_SECTOR)
        {
            res = -EIO;
            goto out;
        }

        // Reserved sector?
        if (entry == 0xFF0 || entry == 0xFF6)
        {
            res = -EIO;
            goto out;
        }

        if (entry == 0x00)
        {
            res = -EIO;
            goto out;
        }

        cluster_to_use = entry;
    }

    res = cluster_to_use;
out:
    return res;
}
static int fat16_read_internal_from_stream(struct disk* disk, struct disk_stream* stream, int cluster, int offset, int total, void* out)
{
    int res = 0;
    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = fat16_get_cluster_for_offset(disk, cluster, offset);
    if (cluster_to_use < 0)
    {
        res = cluster_to_use;
        goto out;
    }


    int offset_from_cluster = offset % size_of_cluster_bytes;

    int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
    int starting_pos = (starting_sector * disk->sector_size) + offset_from_cluster;
    int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;
    res = disk_seek(stream, starting_pos);
    if (res != PEACHOS_ALL_OK)
    {
        goto out;
    }

    res = diskstream_read(stream, out, total_to_read);
    if (res != PEACHOS_ALL_OK)
    {
        goto out;
    }

    total -= total_to_read;
    if (total > 0)
    {
        // We still have more to read
        res = fat16_read_internal_from_stream(disk, stream, cluster, offset+total_to_read, total, out + total_to_read);
    }

out:
    return res;
}

static int fat16_read_internal(struct disk* disk, int starting_cluster, int offset, int total, void* out)
{
    struct fat_private* fs_private = disk->fs_private;
    struct disk_stream* stream = fs_private->cluster_read_stream;
    return fat16_read_internal_from_stream(disk, stream, starting_cluster, offset, total, out);
}

void fat16_free_directory(struct fat_directory* directory)
{
    if (!directory)
    {
        return;
    }

    if (directory->item)
    {
        kfree(directory->item);
    }

    kfree(directory);
}


// function for realesing resources
// that have to do with fat_item
void fat16_fat_item_free(struct fat_item* item)
{
    if (item->type == FAT_ITEM_TYPE_DIRECTORY)
    {
        fat16_free_directory(item->directory);
    }
    else if(item->type == FAT_ITEM_TYPE_FILE)
    {
        kfree(item->item);
    }

    kfree(item);
}

struct fat_directory* fat16_load_fat_directory(struct disk* disk, struct fat_directory_item* item)
{
    int res = 0;
    struct fat_directory* directory = 0;
    struct fat_private* fat_private = disk->fs_private;
    if (!(item->attribute & FAT_FILE_SUBDIRECTORY))
    {
        res = -EINVARG;
        goto out;
    }

    directory = kzalloc(sizeof(struct fat_directory));
    if (!directory)
    {
        res = -ENOMEM;
        goto out;
    }

    int cluster = fat16_get_first_cluster(item);
    int cluster_sector = fat16_cluster_to_sector(fat_private, cluster);
    int total_items = fat16_get_total_items_for_directory(disk, cluster_sector);
    directory->total = total_items;
    directory->sector_pos = fat16_cluster_to_sector(fat_private, cluster);
    int directory_size = directory->total * sizeof(struct fat_directory_item);
    directory->ending_sector_pos = directory->sector_pos + (directory_size / disk->sector_size);

    directory->item = kzalloc(directory_size);
    if (!directory->item)
    {
        res = -ENOMEM;
        goto out;
    }

    res = fat16_read_internal(disk, cluster, 0x00, directory_size, directory->item);
    if (res != PEACHOS_ALL_OK)
    {
        goto out;
    }
    

out:
    if (res != PEACHOS_ALL_OK)
    {
        fat16_free_directory(directory);
    }
    return directory;
}

struct fat_item* fat16_new_fat_item_for_directory_item(struct disk* disk, struct fat_directory_item* item)
{
    struct fat_item* f_item = kzalloc(sizeof(struct fat_item));
    if (!f_item)
    {
        return 0;
    }

    if (item->attribute & FAT_FILE_SUBDIRECTORY)
    {
        f_item->directory = fat16_load_fat_directory(disk, item);
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
    } else {
        f_item->type = FAT_ITEM_TYPE_FILE;
        f_item->item = fat16_clone_directory_item(item, sizeof(struct fat_directory_item));
    }
    return f_item;
}

// function that finds an item in a directory
// and if found returns it
struct fat_item* fat16_find_item_in_directory(struct disk* disk, struct fat_directory* directory, const char* name)
{
    struct fat_item* f_item = 0;
    char tmp_filename[PEACHOS_MAX_PATH];
    for (int i = 0; i < directory->total; i++)
    {
        fat16_get_full_relative_filename(&directory->item[i], tmp_filename, sizeof(tmp_filename));
        if (istrncmp(tmp_filename, name, sizeof(tmp_filename)) == 0)
        {
            // Found it let's create a new fat_item
            f_item = fat16_new_fat_item_for_directory_item(disk, &directory->item[i]);
        }
    }

    return f_item;
}

struct fat_item* fat16_get_directory_entry(struct disk* disk, struct path_part* path)
{
    struct fat_private* fat_private = disk->fs_private;
    struct fat_item* current_item = 0;
    struct fat_item* root_item 
            = fat16_find_item_in_directory(disk, &fat_private->root_directory, path->part);
    if (!root_item)
    {
        goto out;
    }

    struct path_part* next_part = path->next;
    current_item = root_item;
    while(next_part != 0)
    {
        if (current_item->type != FAT_ITEM_TYPE_DIRECTORY)
        {
            current_item = 0;
            break;
        }

        struct fat_item* tmp_item = fat16_find_item_in_directory(disk, current_item->directory, next_part->part);
        fat16_fat_item_free(current_item);
        current_item = tmp_item;
        next_part = next_part->next;
    }
out:
    return current_item;
}

void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode)
{
    // if (mode != FILE_MODE_READ)
    // {
    //     return ERROR(-ERDONLY);
    // }

    struct fat_file_descriptor* descriptor = 0;
    descriptor = kzalloc(sizeof(struct fat_file_descriptor));
    if (!descriptor)
    {
        return ERROR(-ENOMEM);
    }

    // reload the root dir
    struct fat_private* private = disk->fs_private;
    fat16_get_root_directory(disk, private, &private->root_directory);

    descriptor->item = fat16_get_directory_entry(disk, path);
    if (!descriptor->item)
    {
        return ERROR(-EIO);
    }

    descriptor->pos = 0;
    return descriptor;
}

// returns the total ammount of data that was written
// or error if something went wrong
int fat16_write(struct disk* disk, void* descriptor, void* data, uint32_t size, uint32_t offset) {
    int res = 0;
    
    struct fat_file_descriptor* fat_desc = descriptor;
    struct fat_directory_item* item = fat_desc->item->item;
    
    if ((item->attribute & FAT_FILE_SUBDIRECTORY) != 0) {
        res = -EWRITETODIR;
        goto out;
    }


out:
    return res;
}



// returns the ammount of nmembs that were successfully read
int fat16_read(struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr) {
    int res = 0;
    struct fat_file_descriptor* fat_desc = descriptor;
    struct fat_directory_item* item = fat_desc->item->item;
    int offset = fat_desc->pos;
    for (uint32_t i = 0; i < nmemb; i++)
    {
        res = fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size, out_ptr);
        if(ISERROR(res)) 
            goto out;
        out_ptr+=size;
        offset+=size;
    }
    
    res = nmemb;
out:
    return res;
}


int fat16_seek(void *private, uint32_t offset, FILE_SEEK_MODE seek_mode) {
    int res = 0;
    struct fat_file_descriptor* desc = private;
    struct fat_item* desc_item = desc->item;
    if (desc->item->type != FAT_ITEM_TYPE_FILE) {
        res = -EINVARG;
        goto out;
    }
    struct fat_directory_item* ritem = desc_item->item;
    if (offset >= ritem->filesize) {
        res = -EIO;
        goto out;
    }

    switch (seek_mode)
    {
    case SEEK_SET:
        desc->pos = offset;
        break;
    case SEEK_END:
        res = -EUNIMP;
        break;
    case SEEK_CURRENT:
        desc->pos += offset;
    default:
        res = -EINVARG;
        break;
    }

out:
    return res;
}

int fat16_stat(struct disk* disk, void* private, struct file_stat* stat) {
    int res = 0;
    struct fat_file_descriptor* desc = (struct fat_file_descriptor*)private;
    struct fat_item* desc_item = desc->item;
    if(desc_item->type != FAT_ITEM_TYPE_FILE) {
        // only support stating files, not directories
        res = -EINVARG;
        goto out;
    }

    struct fat_directory_item* ritem = desc_item->item;
    stat->file_size = ritem->filesize;
    stat->flags = 0;
    if (ritem->attribute & FAT_FILE_READ_ONLY) {
        stat->flags |= FILE_STAT_READ_ONLY;
    }
out:
    return res;
}

static void fat16_free_file_descriptor(struct fat_file_descriptor* desc) {
    fat16_fat_item_free(desc->item);
    kfree(desc);
}

int fat16_close(void* private) {
    fat16_free_file_descriptor((struct fat_file_descriptor*)private);
    return 0;
}

// returns number of cluster that is free
static int fat16_find_available_cluster_and_allocate(struct disk* disk) {
    int res = 0;
    struct fat_private* private = disk->fs_private;
    struct disk_stream* stream = private->fat_read_stream;

    uint32_t fat_table_position = fat16_get_fat_position(disk);
    // first 3 entries of fat table are skipped
    // times 2 because each entry is two bytes
    disk_seek(stream, fat_table_position + (3 * 2));

    if(!stream) {
        res = -ENOMEM;
        goto out;
    }

    char* buff = (char *)kzalloc(PEACHOS_FAT16_FAT_ENTRY_SIZE);
    diskstream_read(stream, (void *)buff, PEACHOS_FAT16_FAT_ENTRY_SIZE);

    int free_cluster = 4;

    while (1)
    {
        diskstream_read(stream, (void *)buff, 2);
        if (buff[0] == '\0' && buff[1] == '\0')
            break; 
        free_cluster++;
    }

    disk_seek(stream, stream->pos - 2);
    res = diskstream_write(stream, (void *)"FF", 2);
    if (res != PEACHOS_ALL_OK) {
        res = -EIO;
        goto out;
    }

    disk_seek(stream, fat_table_position + (free_cluster* 2));
    diskstream_read(stream, (void *)buff, 2);
    
    kfree(buff);
    res = free_cluster;
out:
    return res;
}

// padds value with space if necessary
void padd_value(char* value, int len, int maxLen) {
    if (len == maxLen)
        return;
    int i = len;
    while (i < maxLen)
    {
        value[i] = ' ';
        i++;
    }
}



int fat16_create(struct disk* disk, char* name, char* ext, int type, struct path_part* path) {
    int res = 0;

    struct fat_private* private = disk->fs_private;
    
    // reload the root dir
    fat16_get_root_directory(disk, private, &private->root_directory);

    struct fat_directory_item* new_item = kzalloc(sizeof(struct fat_directory_item));
    if (!new_item) {
        res = -ENOMEM;
        goto out;
    } 

    // create the item to store in filesystem
    char* name_tmp = kzalloc(sizeof(char) * 8);
    char* ext_tmp = kzalloc(sizeof(char) * 8);
    strcpy(name_tmp, name);
    strcpy(ext_tmp, ext);

    padd_value(name, strlen(name), 8);
    padd_value(ext, strlen(ext), 3);
    strcpy((char *)new_item->filename, name_tmp);
    strcpy((char *)new_item->ext, ext_tmp);
    new_item->attribute = 0;
    new_item->filesize = 0;
    new_item->reserved = 0;

    kfree(name_tmp);
    kfree(ext_tmp);

    // if user wants to create a subdirectory
    if (type == FAT_ITEM_TYPE_DIRECTORY) {
        new_item->attribute |= FAT_FILE_SUBDIRECTORY;
    }

    int available_cluster = fat16_find_available_cluster_and_allocate(disk);
    if (available_cluster < 0) {
        res = -EIO;
        goto out;
    }

    new_item->high_16_bits_first_cluster = (available_cluster >> 16) & 0xFFFF;
    new_item->low_16_bits_first_cluster = available_cluster & 0xFFFF;

    // add item to parent dir
    struct fat_directory* parent_dir = kzalloc(sizeof(struct fat_directory));
    if(!parent_dir) {
        res = -ENOMEM;
        goto out;
    }

    if(!path) {
        parent_dir = &private->root_directory;
    } else {
        struct fat_item* item = fat16_get_directory_entry(disk, path);
        if(!item) {
            res = -EIO;
            goto out;
        }
        if(item->type != FAT_ITEM_TYPE_DIRECTORY) {
            res = -EINVARG;
            goto out;
        }
        parent_dir = item->directory;
    }

    if (parent_dir->total == private->header.primary_header.root_dir_entries) {
        res = -EDIRFULL;
        goto out;
    }

    struct disk_stream* stream = private->directory_stream;
    disk_seek(stream, (parent_dir->sector_pos * private->header.primary_header.bytes_per_sector) + (parent_dir->total * sizeof(struct fat_directory_item)));
    res = diskstream_write(stream, (void *)(new_item), sizeof(struct fat_directory_item));
    if (res != PEACHOS_ALL_OK) {
        res = -EIO;
        goto out;
    }


    kfree(parent_dir);
    kfree(new_item);
out:
    return res;
}