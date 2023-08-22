#include "idt.h"
#include "kernel.h"
#include "config.h"
#include "memory/memory.h"
#include "task/task.h"
#include "../io/io.h"
#include "print/print.h"

// define in idt.asm
// pointer to interrupt handler routines
extern void* interrupt_pointer_table[PEACHOS_TOTAL_INTERRUPTS];

static ISR80H_COMMAND isr80h_commands[PEACHOS_MAX_ISR80H_COMMANDS];

struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

void idt_zero() {
    print("Devide by zero error\n");
}

void no_interrupt_handler() {   
    outb(0x20, 0x20); 
}

void interrupt_handler(int interrupt, struct interrupt_frame* frame) {
    outb(0x20, 0x20); 
}

// method for initializing an entry of the idt
void idt_set(int interrupt_num, void* address) {
    struct idt_desc* desc = &idt_descriptors[interrupt_num];
    desc->offset_1 = (int32_t)address & 0x0000ffff; // lower 16 bits
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;
    desc->type_attr = 0xEE;
    desc->offset_2 = (int32_t)address >> 16; // upper 16 bits
}

// method for initializing the idt
void idt_init() {
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    for (size_t i = 0; i < PEACHOS_TOTAL_INTERRUPTS; i++)
    {
        idt_set(i, interrupt_pointer_table[i]);
    }
    
    idt_set(0, idt_zero);
    idt_set(0x80, isr80h_wrapper);

    idt_load(&idtr_descriptor);
}

void isr80h_register_command(int command_id, ISR80H_COMMAND command) {
    if (command_id < 0 || command_id >= PEACHOS_MAX_ISR80H_COMMANDS)
        panic("isr80h_register_command() command out of bounds\n");
    
    if (isr80h_commands[command_id])
        panic("isr80h_register_command() trying to overwrite existing command\n");

    isr80h_commands[command_id] = command;
}

void* isr80h_handle_command(int command, struct interrupt_frame* frame) {
    void* result = 0;

    if (command < 0 || command >= PEACHOS_MAX_ISR80H_COMMANDS)
        return 0;

    ISR80H_COMMAND command_func = isr80h_commands[command];
    if (!command_func)
        return 0;

    result = command_func(frame);

    return result;
}

void* isr80h_handler(int command, struct interrupt_frame* frame) {
    void* res = 0;
    kernel_page();
    task_current_save_state(frame);
    res = isr80h_handle_command(command, frame);
    task_page();
    return res;
}