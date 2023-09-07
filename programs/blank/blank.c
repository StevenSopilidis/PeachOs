#include "peachos.h"
#include "stdlib.h"

int main(int argc, char** argv) {
    print("hallo from program\n");

    void* ptr = malloc(512);
    if (ptr) {
        free(ptr);
        while(1) {
            if (getKey() != 0) {
                print("Key pressed\n");
            }
        }   
    }
    
    return 0;
}