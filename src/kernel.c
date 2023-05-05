#include "kernel.h"
#include "stdint.h"
#include "stddef.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "disk/stream.h"
#include "fs/pparser.h"
#include "string/string.h"

static uint16_t* video_mem = 0;
static uint16_t terminal_row = 0;
static uint16_t terminal_col = 0;


uint16_t terminal_make_char(char c,char colour)
{
    return (colour << 8) | c;
}

void terminal_putchar(int x, int y, char c, char colour)
{
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, colour);
}

void terminal_writechar(char c, char colour)
{
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
        return;
    }
    terminal_putchar(terminal_col, terminal_row, c, colour);
    terminal_col += 1;
    if (terminal_col == VGA_WIDTH) {
        terminal_col = 0;
        terminal_row++;
    }
}

// clear the terminal 
void terminal_initialize()
{
    video_mem = (uint16_t* )0xb8000;
    terminal_col = 0;
    terminal_row = 0;
    for (int i = 0; i < VGA_HEIGHT; i++)
    {
        for (int j = 0; j < VGA_WIDTH; j++)
        {
            terminal_putchar(j, i, ' ', 0);
        }
    }
    
}

void print(const char* str)
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
    {
        terminal_writechar(str[i], 15);
    }
    
}

extern void problem();
extern void problem2();


static struct paging_4gb_chunk* kernel_chunk = 0;

void kernel_main()
{
    terminal_initialize();
    print("Hallo world\n");

    kheap_init();

    // Search and initialize the disks
    disk_search_and_init();

    // initialize the IDT
    idt_init();

    // Setup paging
    kernel_chunk = paging_new_4gb (PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    
    // switch to kernel pagign chunk
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    // enable paging
    enable_paging();

    struct disk_stream* stream = diskstream_new(0);
    disk_seek(stream, 0x201);
    unsigned char c = 's';
    diskstream_read(stream, &c, 1);

    // Enable the system interupts
    enable_interupts();
}   