#ifndef _DISK_STREAMER_H
#define _DISK_STREAMER_H

#include "disk.h"

struct disk_stream {
    int pos; // position we are at the stream
    struct disk* disk; // disk we bind the stream to
}


#endif