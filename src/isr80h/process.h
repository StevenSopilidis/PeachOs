#ifndef _ISR80H_PROCESS_H_
#define _ISR80H_PROCESS_H_

struct interrupt_frame;
void* isr80h_command6_process_load_start(struct interrupt_frame* frame);

#endif