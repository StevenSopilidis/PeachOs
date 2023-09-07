#ifndef _ISR80H_HEAP_H_
#define _ISR80H_HEAP_H_

#include "idt/idt.h"

void* isr80h_command4_malloc(struct interrupt_frame* frame);
void* isr80h_command5_free(struct interrupt_frame* frame);

#endif