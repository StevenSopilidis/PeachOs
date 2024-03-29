#ifndef _PRINT_H_
#define _PRINT_H_

#define VGA_WIDTH 80
#define VGA_HEIGHT 20

void terminal_initialize();
void print(const char* str);
void printf(const char* format, ...);
void terminal_writechar(char c, char colour);

#endif