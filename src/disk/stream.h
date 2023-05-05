#ifndef _DISK_STREAM_H
#define _DISK_STREAM_H

#include "disk.h"

struct disk_stream {
    int pos; // position we are at the stream
    struct disk* disk; // disk we bind the stream to
};

struct disk_stream* diskstream_new(int disk_id);
int disk_seek(struct disk_stream* stream, int pos); 
int diskstream_read(struct disk_stream* stream, void* out, int total);
void diskstream_close(struct disk_stream* stream);


#endif