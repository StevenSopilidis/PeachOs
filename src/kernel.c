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
#include "isr80h/isr80h.h"
#include "print/print.h"
#include "keyboard/keyboard.h"

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

    // register kernel commands
    isr80h_register_commands();

    // init keyboard
    keyboard_init();

    struct process* process = 0;
    int res = process_load_switch("0:/blank.elf", &process);
    if (res != PEACHOS_ALL_OK)
        panic("Failed to load blank.elf\n");

    task_run_first_ever_task();

    terminal_initialize();


out:
    while(1) {}
}   