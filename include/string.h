/*
 * string.h - Simple string utilities
 */

#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char* str);
int strncmp(const char* s1, const char* s2, size_t n);
char* strncpy(char* dest, const char* src, size_t n);
int snprintf(char* str, size_t size, const char* format, ...);

#endif