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
#include "fs/file.h"

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


static struct paging_4gb_chunk* kernel_chunk = 0;

void kernel_main()
{
    terminal_initialize();
    print("Hallo world\n");

    kheap_init();

    // initialize filesystems
    // fs_init();

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

    // Enable the system interupts
    enable_interupts();

    char* d = "hallo steven asd fnoiwerio 4230hr430hj3hir43iowedfoji  dsfjiowehrjiowtrhw   wetoihjwetio";
    uint32_t addr = (uint32_t)d; 
    disk_write_block(disk_get(0),300, 1, addr);
    struct disk_stream* stream = diskstream_new(0);
    disk_seek(stream, 512 * 300 + 5);
    diskstream_write(stream, (unsigned char* )"not hallo", 9);
    disk_seek(stream, 512 * 300 + 30);
    diskstream_write(stream, (unsigned char* )"dick", 4);
    disk_seek(stream, 512 * 300 + 510);
    diskstream_write(stream, (unsigned char* )"Easter", 6);
    disk_seek(stream, 512 * 300);
    char* buf = kzalloc(200);
    diskstream_read(stream, (void *)buf, 89);
    disk_seek(stream, 512 * 301);
    diskstream_read(stream, (void *)buf, 89);

    // int fd = fopen("0:/hello.txt", "r");
    // if(fd) {
    //     struct file_stat s;
    //     fstat(fd, &s);
    //     fclose(fd);

    //     print("File closed\n");
    // }

    while(1) {}
}   