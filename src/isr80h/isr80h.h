#ifndef _ISR80H_H_
#define _ISR80H_H_

enum SystemCommands {
    SYSTEM_COMMAND0_SUM,
    SYSTEM_COMMAND1_PRINT,
    SYSTEM_COMMAND2_GET_KEY,
    SYSTEM_COMMAND3_PUTCHAR,   
};
void isr80h_register_commands();

#endif