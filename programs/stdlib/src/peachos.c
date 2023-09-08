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

        // Carriage return means we have read the line
        if (key == 13)
        {
            break;
        }

        if (display)
        {
            peachosPutChar(key);
        }

        // Backspace
        if (key == 0x08 && i >= 1)
        {
            out[i-1] = 0x00;
            // -2 because we will +1 when we go to the continue
            i -= 2;
            continue;
        }

        out[i] = key;
    }
}