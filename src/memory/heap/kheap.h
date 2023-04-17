#ifndef _KHEAP_H
#define _KHEAP_H
#include "stdint.h"
#include "stddef.h"

void kheap_init();
void* kmalloc(size_t size);

#endif