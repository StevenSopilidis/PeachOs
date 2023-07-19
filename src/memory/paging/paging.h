#ifndef _PAGING_H_
#define _PAGING_H_

#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

// cache disabled bit in page directory entry
#define PAGING_CACHE_DISABLED  0b00010000
// write through bit in page directory entry
#define PAGING_WRITE_THROUGH   0b00001000
// User/Supervisor bit in page directory entry
#define PAGING_ACCESS_FROM_ALL 0b00000100
// Write bit in page directory entry
#define PAGING_IS_WRITEABLE    0b00000010
// Present bit in page directory entry
#define PAGING_IS_PRESENT      0b00000001

// total entries per page table & page
#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024 

// size of a page
#define PAGING_PAGE_SIZE 4096

// represents a 4gb virtual address
struct paging_4gb_chunk
{
    // pointer to our page directory
    uint32_t* directory_entry;
};

struct paging_4gb_chunk* paging_new_4gb(uint8_t flags);
void paging_free_4gb(struct paging_4gb_chunk* chunk);
void paging_switch(struct paging_4gb_chunk* directory);
uint32_t* paging_4gb_chunk_get_directory(struct paging_4gb_chunk* chunk);
// before enable paging we need
// 1) create the 4gb chunk
// 2) switch directory to the one we created above
extern void enable_paging();

int paging_set(uint32_t* directory, void* virtual_address, uint32_t value);
bool paging_is_aligned(void* addr);

int paging_map_to(struct paging_4gb_chunk* directory, void* vaddr, void* paddr, void* paddr_end, int flags);
int paging_map_range(struct paging_4gb_chunk* directory, void* vaddr, void* paddr, int count, int flags);
int paging_map(struct paging_4gb_chunk* directory, void* vaddr, void* paddr, int flags);


void* paging_align_address(void *ptr);

#endif