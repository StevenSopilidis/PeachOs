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

char* strcpy(char* dest, const char* src) {
    char* res = dest;
    while (*src != '\0')
    {
        *dest = *src;
        src++;
        dest++;
    }
    *dest = '\0';

    return res;
}