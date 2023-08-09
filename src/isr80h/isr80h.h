#ifndef _ISR80H_H_
#define _ISR80H_H_

enum SystemCommands {
    SYSTEM_COMMAND0_SUM,
    SYSTEM_COMMAND1_PRINT,
};
void isr80h_register_commands();

#endif