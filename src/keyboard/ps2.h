#ifndef _PS2_H_
#define _PS2_H_

#define PS2_PORT 0x64
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE

#define PS2_KEYBORAD_KEY_RELEASED 0x80
#define ISR_KEYBOARD_INTERRUPT 0x21

#define KEYBOARD_INPUT_PORT 0x60

struct keyboard* ps2_init();

#endif