#include "idt.h"
#include "kernel.h"
#include "config.h"
#include "memory/memory.h"
#include "../io/io.h"

struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

void idt_zero() {
    print("Devide by zero error\n");
}

void int21h_handler() {
    print("Keyboard pressed\n");
    // acknowledge interupt from PIC
    outb(0x20, 0x20); 
}

void no_interrupt_handler() {   
    outb(0x20, 0x20); 
}

// method for initializing an entrie of the idt
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
        idt_set(i, no_interrupt);
    }
    idt_set(0, idt_zero);
    idt_set(0x21, int21h);

    idt_load(&idtr_descriptor);
}