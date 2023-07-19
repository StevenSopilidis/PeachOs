#include "kernel.h"
#include "stdint.h"
#include "stddef.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "disk/stream.h"
#include "fs/pparser.h"
#include "string/string.h"
#include "fs/file.h"
#include "gdt/gdt.h"
#include "task/tss.h"
#include "config.h"
#include "task/task.h"
#include "task/process.h"
#include "status.h"

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

void panic(const char* msg) {
    print(msg);
    while(1) {}
}

// switches to page directory to kernel page directory
// and restore kernel segment
void kernel_page() {
    kernel_registers();
    paging_switch(kernel_chunk);
}

struct tss tss;

struct gdt gdt_real[PEACHOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[PEACHOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},                // NULL Segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a},           // Kernel code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92},            // Kernel data segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf8},              // User code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2},             // User data segment
    {.base = (uint32_t)&tss, .limit=sizeof(tss), .type = 0xE9}      // TSS Segment
};

void kernel_main()
{
    terminal_initialize();

    memset(gdt_real, 0, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, PEACHOS_TOTAL_GDT_SEGMENTS);
    gdt_load(gdt_real, sizeof(gdt_real));

    kheap_init();

    // initialize filesystems
    fs_init();

    // Search and initialize the disks
    disk_search_and_init();

    // initialize the IDT
    idt_init();

    // setup tss
    memset(&tss, 0x00, sizeof(tss));
    tss.esp0 = 0x600000;
    tss.ss0 = KERNEL_DATA_SELECTOR;

    // load the tss
    tss_load(0x28);

    // Setup paging
    kernel_chunk = paging_new_4gb (PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    
    // switch to kernel pagign chunk
    paging_switch(kernel_chunk);
    // enable paging
    enable_paging();

    // struct process* process = 0;
    // int res = process_load("0:/blank.bin", &process);
    // if (res != PEACHOS_ALL_OK)
    //     panic("Failed to load blank.bin\n");

    // task_run_first_ever_task();

    int res = fcreate("dev", "bin", FS_FILE, "0:/");
    if(res != 0) {
        print("Could not create file");
    }

    // res = fopen("0:/hello.txt", "r");
    // if(!res)
    //     print("Could not open file");

    while(1) {}
}   