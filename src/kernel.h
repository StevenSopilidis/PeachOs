#ifndef _KERNEL_H
#define _KERNEL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 20

void kernel_main();
void print(const char* str);

#define ERROR(value) (void*)(value)
#define ERROR_I(value) (int)(value)
#define ISERROR(value) ((int)value < 0)

#endif