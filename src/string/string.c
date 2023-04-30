#include "string.h"

int strlen(const char* ptr) {
    int count = 0;
    while (*(ptr++) != '\0') {
        count++;
    }
    return count;    
}

int strnlen(const char* ptr, int max) {
    int count = 0;
    while (*(ptr++) != '\0' && count < max) {
        count++;
    }
    return count;    
}


bool is_digit(char c) {
    return c >= 48 && c <= 57;
}

int to_numeric_digit(char c) {
    return c - 48;
}