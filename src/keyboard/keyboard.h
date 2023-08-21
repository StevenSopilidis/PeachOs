#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "task/process.h"
#include "task/task.h"


typedef int (*KEYBOARD_INIT_FUNCTION)();

struct keyboard {
    KEYBOARD_INIT_FUNCTION init;
    char name[20];
    struct keyboard* next;
};

void keyboard_init();
void keyboard_backspace(struct process* process);
void keyboard_push(char c);
char keyboard_pop();
int keyboard_insert(struct keyboard* keyboard);

#endif