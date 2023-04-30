#ifndef _STRING_H_
#define _STRING_H_

#include "stdbool.h"

// function that will return the length of a string
int strlen(const char* ptr);

// function that will return the length of a string
// counting only up to max
int strnlen(const char* ptr, int max);

// checks wether char c is a valid digit
bool is_digit(char c);
// gets a numeric input like '5' and returns 5 
int to_numeric_digit(char c);


#endif