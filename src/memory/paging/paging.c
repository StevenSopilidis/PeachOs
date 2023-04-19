#include "paging.h"
#include "../heap/kheap.h"
#include "status.h"

extern void paging_load_directory(uint32_t* directory);

// current page directory
static uint32_t* current_directory = 0;

// creates the page directory & page tables
struct paging_4gb_chunk* paging_new_4gb(uint8_t flags)
{
    // allocate size for the new page directory
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0; // offset in real address
    // setup page directory entries
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        // create the page table
        uint32_t* entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        // setup page table entries
        for (int j = 0; j < PAGING_TOTAL_ENTRIES_PER_TABLE; j++)
        {
            entry[j] = (offset + (j * PAGING_PAGE_SIZE)) | flags; 
        }
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
        directory[i] = (uint32_t)entry | flags | PAGING_IS_WRITEABLE;
    }   

    struct paging_4gb_chunk* chunk = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk->directory_entry = directory;
    return chunk;
}

// method for getting the page directory out of the 4gb chunk
uint32_t* paging_4gb_chunk_get_directory(struct paging_4gb_chunk* chunk)
{
    return chunk->directory_entry;
}

// method for swapping page directories
void paging_switch(uint32_t* directory)
{
    paging_load_directory(directory);
    current_directory = directory;
}

// method for checking if virtual address is aligned
bool paging_is_aligned(void* addr)
{
    return ((uint32_t)addr % PAGING_PAGE_SIZE) == 0;
}

/*  gets virtual address and outputs the index of 
    the page directory & page table that this address
    is located on 
    for e.x virtual address is 0x405000
    dir_index = 0x405000 / (1024 * 4096) == 1 each page table can manage 1024 * 4096 bytes of memory
    table_index = 0x405000 % (1024 * 4096) / 4096
    so for the first 1024 * 4096 bytes of mem are handled by first page table
    and each one of the 4096 chunks is handled by the page table entry
    once we go 1024 * 4096 + 1 this is handled by the second page table
    which will handle address from 1024 * 4096 + 1 .... 2 * (1024 * 4096)
*/
int paging_get_indexes(void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index_out)
{
    int res = PEACHOS_ALL_OK;
    if(!paging_is_aligned(virtual_address)) {
        res = -EINVARG;
        goto out;
    }
    
    *directory_index_out = ((uint32_t)virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *table_index_out = ((uint32_t)virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);
out:
    return res;
}   

// will set page_table entry to value provided
// so it will essentially we can map a virtual address to 
// a desired physical address
int paging_set(uint32_t* directory, void* virtual_address, uint32_t value)
{
    if (!paging_is_aligned(virtual_address))
        return -EINVARG;
    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    int res = paging_get_indexes(virtual_address, &directory_index, &table_index);
    if (res < 0)
        return res;
    uint32_t entry = directory[directory_index];
    // extract the page table address from the page directory entry
    uint32_t* table = (uint32_t*)(entry & 0xfffff000);
    table[table_index] = value;
    return PEACHOS_ALL_OK; 
}