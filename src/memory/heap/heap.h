#ifndef _HEAP_H
#define _HEAP_H

#include "../../config.h"
#include "stdint.h"
#include "stddef.h"

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0x00

// mask to get the bit that indicates wether the allocation
// is continued to the next block on heap
#define HEAP_BLOCK_HAS_NEXT 0b10000000 
// mask to get the bit that indicates wether the block 
// is the first of the allocation
#define HEAP_BLOCK_IS_FIRST 0b01000000

// 8 bit entry on our heap table
typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

struct heap_table 
{
    HEAP_BLOCK_TABLE_ENTRY* entries; // start address of heap table entries
    size_t total; // number of entries in our table
};

struct heap
{
    struct heap_table* table;
    void* saddr; // start address of our heap data pool
};

// Method for creating the heap
// returns 0 if heap creation was successfull
// heap: our heap
// ptr: start of our heap data pool
// end: end of our heap data pool
// table: our heap table 
int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table);

void* heap_malloc(struct heap* heap, size_t size);
void heap_free(struct heap* heap, void* ptr);

#endif