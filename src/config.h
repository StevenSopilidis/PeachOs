#ifndef _CONFIG_H
#define _CONFIG_H

// max length of path in our OS
#define PEACHOS_MAX_PATH 108

#define PEACHOS_TOTAL_INTERRUPTS 512
#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

// size of our kernel heap (100 MB)
#define PEACHOS_HEAP_SIZE_BYTES 104857600
// size of our heap entries
#define PEACHOS_HEAP_BLOCK_SIZE 4096
// start address of our heap data pool
#define PEACHOS_HEAP_ADDRESS 0x01000000
// address of our heap table
#define PEACHOS_HEAP_TABLE_ADDRESS 0x00007E00

// sector size we use
#define PEACHOS_SECTOR_SIZE 512

// max number of file systems than can be used in our os
#define PEACHOS_MAX_FILESYSTEMS 12
#define PEACHOS_MAX_FILE_DESCRIPTORS 512

#define PEACHOS_TOTAL_GDT_SEGMENTS 6

#define PEACHOS_PROGRAM_VIRTUAL_ADDRESS 0x400000
#define PEACHOS_USER_PROGRAM_STACK_SIZE 1024 * 16
#define PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START 0x3FF000
#define PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END  PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START - PEACHOS_USER_PROGRAM_STACK_SIZE
#define PEACHOS_MAX_PROGRAM_ALLOCATIONS 1024
#define PEACHOS_MAX_PROCESSES 12

#define USER_DATA_SEGMENT 0x23
#define USER_CODE_SEGMENT 0x1b

#define PEACHOS_MAX_ISR80H_COMMANDS 1024


#endif
