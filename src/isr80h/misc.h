#ifndef _ISR80H__MISC_H_
#define _ISR80H__MISC_H_

struct interrupt_frame;
void* isr80h_command0_sum(struct interrupt_frame* frame);

#endif