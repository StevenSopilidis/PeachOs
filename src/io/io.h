#ifndef _IO_H_
#define _IO_H_

#include "stdint.h"

// read byte from specified port
extern unsigned char insb(unsigned short port);
// read word from specified port
extern unsigned short insw(unsigned short port);

// write byte to specified port
extern void outb(unsigned short port, unsigned char val);
// read word to specified port
extern void outw(unsigned short port, unsigned short val);

#endif