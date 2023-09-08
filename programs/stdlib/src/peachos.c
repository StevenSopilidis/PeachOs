#include "peachos.h"

// gets key and blocks until it does
int peachos_getKey_block() {
    int val = peachos_getKey();
    while (val == 0) {
        val = peachos_getKey();
    }
    return val;
}

// function for getting a line and printing it
// @param out: output buffer
// @param max: max buffer size
// @param display: wether or not to display to screen 
void peachos_terminal_readline(char* out, int max, bool display) {
    int i = 0;
    for (i = 0; i < max -1; i++)
    {
        char key = peachos_getKey_block();

        if (key == 13)
            break;

        if (key == 8 && i >= 1) {
            out[i - 1] = 0;
            i -= 2;
            continue;
        }

        out[i] = key;
        if (display)
            peachosPutChar(key);
    }

    // Add the null terminator
    out[i] = 0x00;
}