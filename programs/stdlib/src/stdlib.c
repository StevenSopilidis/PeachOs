#include "stdlib.h"
#include "peachos.h"

void* malloc(size_t size) {
    return peachosMalloc(size);
}

void free(void* ptr){
    peachosFree(ptr);
}