#include "kernel.h"
#include "stdint.h"
#include "stddef.h"
#include "idt/idt.h"
#include "io/io.h"

static uint16_t* video_mem = 0;
static uint16_t terminal_row = 0;
static uint16_t terminal_col = 0;


uint16_t terminal_make_char(char c,char colour)
{
    return (colour << 8) | c;
}

void terminal_putchar(int x, int y, char c, char colour)
{
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, colour);
}

void terminal_writechar(char c, char colour)
{
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
        return;
    }
    terminal_putchar(terminal_col, terminal_row, c, colour);
    terminal_col += 1;
    if (terminal_col == VGA_WIDTH) {
        terminal_col = 0;
        terminal_row++;
    }
}

// clear the terminal 
void terminal_initialize()
{
    video_mem = (uint16_t* )0xb8000;
    terminal_col = 0;
    terminal_row = 0;
    for (int i = 0; i < VGA_HEIGHT; i++)
    {
        for (int j = 0; j < VGA_WIDTH; j++)
        {
            terminal_putchar(j, i, ' ', 0);
        }
    }
    
}

size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len++]) {}
    return len;
}

void print(const char* str)
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
    {
        terminal_writechar(str[i], 15);
    }
    
}

extern void problem();
extern void problem2();


void kernel_main()
{
    terminal_initialize();
    print("Hallo world\n");

    // initialize the IDT
    idt_init();

    print("Hallo world2\n");
}