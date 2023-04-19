#ifndef _IDT_H
#define _IDT_H

#include "stdint.h"

// IDT entry descriptor
struct idt_desc {
    uint16_t offset_1; // 0-15 offset bits
    uint16_t selector; // gdt selector
    uint8_t zero; // unused byte
    uint8_t type_attr; // includes P, DPL, storage segment and Type
    uint16_t offset_2; // 16-31 offset bits
} __attribute__((packed));

// descriptor of idt table
struct idtr_desc {
    uint16_t limit; // size of the descriptor table -1 
    uint32_t base; // base address of the start of the IDT
} __attribute__((packed));

extern void idt_load(struct idtr_desc* ptr);
extern void int21h();
extern void no_interrupt();
extern void enable_interupts();
extern void disable_interupts();
void idt_init();

#endif