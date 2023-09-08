#include "shell.h"
#include "peachos.h"

int main(int argc, char** argv) {
    print("PeachOS v.1.0.0\n");

    while (1){
        print(">");
        char buf[1024];
        peachos_terminal_readline(buf, sizeof(buf), true);
        print("\n");
    }
    

    return 0;
}