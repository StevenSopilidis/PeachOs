#ifndef _PEACHOS_H_
#define _PEACHOS_H_

#include "stddef.h"

void print(const char* message);
int getKey();
void* peachosMalloc(size_t size);
void peachosFree(void* ptr);

#endif