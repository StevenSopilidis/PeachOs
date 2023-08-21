#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "stdint.h"
#include "config.h"
#include "task.h"

struct process {
    // process id
    uint16_t id;

    char filename[PEACHOS_MAX_PATH];

    // the main process task
    struct task* task;
    // allocations of a process
    // when process fails or stops executing
    // we can free all the memory to avoid memory leaks
    void* allocations[PEACHOS_MAX_PROGRAM_ALLOCATIONS];
    
    // physical pointer to process memory
    void* ptr;

    // physical pointer to stack memory
    void* stack;

    // the size of the data pointed to by "ptr"
    uint32_t size;

    struct keyboard_buffer {
        char buffer[PEACHOS_KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    } keyboard;
};

int process_load_for_slot(const char* filename, struct process** process, int process_slot);
int process_load(const char* filename, struct process** process);

struct process* process_current();
struct process* process_get(int process_id);

#endif