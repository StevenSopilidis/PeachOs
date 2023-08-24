#ifndef _IDT_H
#define _IDT_H

#include "stdint.h"

struct interrupt_frame;
typedef void*(*ISR80H_COMMAND)(struct interrupt_frame* frame);
typedef void(*INTERRUPT_CALLBACK_FUNCTION)();

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

struct interrupt_frame
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t reserved;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

extern void idt_load(struct idtr_desc* ptr);
extern void int21h();
extern void no_interrupt();
extern void enable_interupts();
extern void disable_interupts();
extern void isr80h_wrapper();

void idt_init();
void isr80h_register_command(int command_id, ISR80H_COMMAND command);
void interrupt_handler(int interrupt, struct interrupt_frame* frame);
int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback);

#endif