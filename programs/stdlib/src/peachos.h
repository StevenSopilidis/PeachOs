#ifndef _PEACHOS_H_
#define _PEACHOS_H_

#include "stddef.h"
#include "stdbool.h"

void print(const char* message);
void peachosPutChar(char c);
int peachos_getKey();
int peachos_getKey_block();
void peachos_terminal_readline(char* out, int max, bool display);
void* peachos_malloc(size_t size);
void peachos_free(void* ptr);
void peachos_process_load_start(const char* filename);

#endif