#ifndef _PS2_H_
#define _PS2_H_

#define PS2_PORT 0x64
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE

struct keyboard* ps2_init();

#endif