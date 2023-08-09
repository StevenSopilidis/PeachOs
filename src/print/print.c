#include "stddef.h"
#include "stdint.h"
#include "stdarg.h"
#include "print.h"
#include "memory/memory.h"
#include "string/string.h"
#include "memory/heap/kheap.h"

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

    if (terminal_row >= VGA_HEIGHT) {
        for (size_t i = 1; i < VGA_HEIGHT; i++)
        {
            memcpy(video_mem + (i - 1) * VGA_WIDTH, video_mem + i * VGA_WIDTH, VGA_WIDTH);
        }
        terminal_row--;
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

void print(const char* str)
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
    {
        terminal_writechar(str[i], 15);
    }
}

static int hex_to_string(char* buffer, int position, uint32_t digits) {
    char digits_buffer[25];
    char digits_map[16] = "0123456789ABCDEF";
    int size = 0;

    do {
        digits_buffer[size++] = digits_map[digits % 16];
        digits /= 16;
    } while (digits != 0);

    for (int i = size-1; i >= 0; i--) {
        buffer[position++] = digits_buffer[i];
    }

    buffer[position++] = 'H';

    return size+1;
}

static int read_string(char *buffer, int position, const char* string) {
    int index = 0;
    for (index = 0; string[index] != '\0'; index++) {
        buffer[position++] = string[index];
    }
    return index;
}

static int udecimal_to_string(char *buffer,int position,uint32_t digits) {
    char digits_map[10] = "0123456789";
    char digits_buffer[25];
    int size = 0;

    do {
        digits_buffer[size++] = digits_map[digits % 10];
        digits /= 10;
    } while (digits != 0);

    for (int i = size-1; i >= 0; i--) {
        buffer[position++] = digits_buffer[i];
    }

    return size;
}

static int decimal_to_string(char *buffer,int position, int32_t digits) {
    int size = 0;
    if (digits < 0) {
        digits = -digits;
        buffer[position++] = '-';
        size++;
    }

    size += udecimal_to_string(buffer, position, (uint32_t)digits);
    return size;
}


void printf(const char* format, ...) {
    char* buffer = kzalloc(strlen(format));
    int buffer_size = 0;
    memset(buffer, 0, strlen(format));
    int32_t integer = 0;
    char *string = 0;
    double num = 0.f;

    va_list args;
    va_start(args, format);
    for (size_t i = 0; format[i] != '\0'; i++)
    {
        if (format[i] != '%') {
            buffer[buffer_size++] = format[i];
            continue;
        }
        switch (format[++i])
        {
            case 'x': // hex
                integer = va_arg(args, int32_t);
                buffer_size += hex_to_string(buffer, buffer_size, (uint32_t)integer);
                break;
            case 'u': // unsigned decimal
                integer = va_arg(args, int32_t);
                buffer_size += udecimal_to_string(buffer, buffer_size, (uint32_t)integer);
                break;
            case 'd': // signed decimal
                integer = va_arg(args, int32_t);
                buffer_size += decimal_to_string(buffer, buffer_size, integer);
                break;
            case 's': // string
                string = va_arg(args, char*);
                buffer_size += read_string(buffer, buffer_size, string);
                break;
            default:
                buffer[buffer_size++] = '%';
                i--;
                break;
        }
    }
    print(buffer);
}