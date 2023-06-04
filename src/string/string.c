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

// returns 0 if strings are the same
// else returns the difference between the 
// first different chars
int strncmp(const char* str1, const char* str2, int n) {
    unsigned char u1, u2;
    while(n-- > 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2) 
            return u1 - u2;
        if (u1 == '\0')
            return 0; 
    }
    return 0;
}

// reads string untill finds \0 or terminator character
// or we hit max
int strnlen_terminator(const char* str, int max, char terminator) {
    int i = 0;
    for (i = 0; i < max; i++)
        if(str[i] == '\0' || str[i] == terminator) 
            break;
    return i;
}

// function for converting char to lowercase
char tolower(char c) {
    // 65 ... 90 upper case letters in ASCII
    // +32 goes to the lower case equivelant in ASCII
    if (c >= 65 && c <= 90) {
        c += 32;
    }
    return c;
}

// string compare function that is case insensitive
int istrncmp(const char* str1, const char* str2, int n) {
    unsigned char u1, u2;
    while (n-- > 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if(u1 != u2 && tolower(u1) != tolower(u2)) {
            return u1 - u2;
        } if (u1 == '\0')
            return 0;
    }
    return 0;
}