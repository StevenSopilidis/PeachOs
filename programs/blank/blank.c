#include "peachos.h"
#include "stdlib.h"
#include "stdbool.h"
#include "string.h"

int main(int argc, char** argv) {
    char* ptr = malloc(20);
    strcpy(ptr, "hallo world");

    print(ptr);
    free(ptr);

    while (1) {
        
    }
    return 0;
}