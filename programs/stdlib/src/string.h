#ifndef _PEACHOS_STRING_H_
#define _PEACHOS_STRING_H_

#include "stdbool.h"

int strlen(const char* ptr);
int strnlen(const char* ptr, int max);
bool is_digit(char c);
int to_numeric_digit(char c);
char* strcpy(char* dest, const char* src);
char* strncpy(char *dest, const char* src, int count);
int strncmp(const char* str1, const char* str2, int n);
int strnlen_terminator(const char* str, int max, char terminator);
char tolower(char c);
int istrncmp(const char* str1, const char* str2, int n);
char* strtok(char* str, const char* delimiters)


#endif