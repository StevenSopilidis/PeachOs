#include "stream.h"
#include "../memory/heap/kheap.h"
#include "../config.h"
#include "kernel.h"
#include "string/string.h"
#include "stdbool.h"

struct disk_stream *diskstream_new(int disk_id)
{
    struct disk *disk = disk_get(disk_id);
    if (!disk)
    {
        return 0;
    }

    struct disk_stream *streamer = kzalloc(sizeof(struct disk_stream));
    streamer->pos = 0;
    streamer->disk = disk;
    return streamer;
}

int disk_seek(struct disk_stream *stream, int pos)
{
    stream->pos = pos;
    return 0;
}

// @stream: disk stream where to write data
// @data: data to write to disk
// @total: number of data to write
int diskstream_write(struct disk_stream *stream, void *data, int total)
{
    int res = 0;
    int sectors_to_read = total / PEACHOS_SECTOR_SIZE;
    int starting_sector = stream->pos / PEACHOS_SECTOR_SIZE;
    int offset = stream->pos % PEACHOS_SECTOR_SIZE;

    if (total % PEACHOS_SECTOR_SIZE != 0) 
        sectors_to_read++;

    if (offset + total >= PEACHOS_SECTOR_SIZE)
        sectors_to_read++;

    char* buff = kzalloc(sectors_to_read * PEACHOS_SECTOR_SIZE);
    char* tmp = buff;

    for (int i = 0; i < sectors_to_read; i++)
    {
        disk_read_block(stream->disk, starting_sector + i, 1, (void *)tmp);
        tmp += PEACHOS_SECTOR_SIZE;
    }

    for (int i = offset; i < total + offset; i++)
    {   
        *(char *)(buff + i) = *(char *)(data++);
    }
    
    for (int i = 0; i < sectors_to_read; i++)
    {
        disk_write_block(stream->disk, starting_sector + i, 1, (uint32_t)buff);
        buff += PEACHOS_SECTOR_SIZE;
    }
    
     
out:
    kfree(buff);
    return res;
}

int diskstream_read(struct disk_stream *stream, void *out, int total)
{
    int sector = stream->pos / PEACHOS_SECTOR_SIZE;
    int offset = stream->pos % PEACHOS_SECTOR_SIZE;
    int total_to_read = total;
    bool overflow = (offset + total_to_read) >= PEACHOS_SECTOR_SIZE;
    char buf[PEACHOS_SECTOR_SIZE];

    if (overflow)
    {
        total_to_read -= (offset + total_to_read) - PEACHOS_SECTOR_SIZE;
    }

    int res = disk_read_block(stream->disk, sector, 1, buf);
    if (res < 0)
    {
        goto out;
    }

    for (int i = 0; i < total_to_read; i++)
    {
        *(char *)out++ = buf[offset + i];
    }
    // Adjust the stream
    stream->pos += total_to_read;
    if (overflow)
    {
        res = diskstream_read(stream, out, total - total_to_read);
    }
out:
    return res;
}

void diskstream_close(struct disk_stream *stream)
{
    kfree(stream);
}