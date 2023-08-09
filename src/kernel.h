#ifndef _KERNEL_H
#define _KERNEL_H

void kernel_main();
void panic(const char* msg);

void kernel_page();
void kernel_registers();

#define ERROR(value) (void*)(value)
#define ERROR_I(value) (int)(value)
#define ISERROR(value) ((int)value < 0)

#endif