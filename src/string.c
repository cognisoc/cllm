/*
 * string.c - Simple string utilities implementation
 */

#include "string.h"
#include <stdarg.h>

// Simple implementation of strlen
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

// Simple implementation of strncmp
int strncmp(const char* s1, const char* s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return s1[i] - s2[i];
        }
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

// Simple implementation of strncpy
char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

// Simple implementation of snprintf (limited functionality)
int snprintf(char* str, size_t size, const char* format, ...) {
    if (size == 0) return 0;
    
    va_list args;
    va_start(args, format);
    
    size_t i = 0;
    size_t format_i = 0;
    
    while (format[format_i] != '\0' && i < size - 1) {
        if (format[format_i] == '%' && format[format_i + 1] != '\0') {
            format_i++;
            if (format[format_i] == 's') {
                // String argument
                char* arg = va_arg(args, char*);
                size_t arg_len = strlen(arg);
                size_t copy_len = (i + arg_len < size - 1) ? arg_len : (size - 1 - i);
                strncpy(str + i, arg, copy_len);
                i += copy_len;
            } else if (format[format_i] == 'd' || format[format_i] == 'i') {
                // Integer argument
                int arg = va_arg(args, int);
                // Convert integer to string
                char int_str[12]; // Enough for 32-bit integer
                int pos = 0;
                int temp = arg;
                
                // Handle negative numbers
                if (arg < 0) {
                    if (i < size - 1) {
                        str[i++] = '-';
                    }
                    temp = -temp;
                }
                
                // Convert to string (reverse order)
                if (temp == 0) {
                    int_str[pos++] = '0';
                } else {
                    while (temp > 0 && pos < 11) {
                        int_str[pos++] = '0' + (temp % 10);
                        temp /= 10;
                    }
                }
                
                // Copy in correct order
                for (int j = pos - 1; j >= 0 && i < size - 1; j--) {
                    str[i++] = int_str[j];
                }
            } else if (format[format_i] == 'u') {
                // Unsigned integer argument
                unsigned int arg = va_arg(args, unsigned int);
                // Convert unsigned integer to string
                char int_str[12]; // Enough for 32-bit unsigned integer
                int pos = 0;
                unsigned int temp = arg;
                
                // Convert to string (reverse order)
                if (temp == 0) {
                    int_str[pos++] = '0';
                } else {
                    while (temp > 0 && pos < 11) {
                        int_str[pos++] = '0' + (temp % 10);
                        temp /= 10;
                    }
                }
                
                // Copy in correct order
                for (int j = pos - 1; j >= 0 && i < size - 1; j--) {
                    str[i++] = int_str[j];
                }
            } else if (format[format_i] == 'z' && format[format_i + 1] == 'u') {
                // size_t argument
                format_i++; // Skip 'z'
                size_t arg = va_arg(args, size_t);
                // Convert size_t to string
                char int_str[24]; // Enough for 64-bit size_t
                int pos = 0;
                size_t temp = arg;
                
                // Convert to string (reverse order)
                if (temp == 0) {
                    int_str[pos++] = '0';
                } else {
                    while (temp > 0 && pos < 23) {
                        int_str[pos++] = '0' + (temp % 10);
                        temp /= 10;
                    }
                }
                
                // Copy in correct order
                for (int j = pos - 1; j >= 0 && i < size - 1; j--) {
                    str[i++] = int_str[j];
                }
            } else {
                // Unknown format specifier, just copy it
                if (i < size - 1) {
                    str[i++] = '%';
                }
                if (i < size - 1) {
                    str[i++] = format[format_i];
                }
            }
            format_i++;
        } else {
            // Regular character
            str[i++] = format[format_i++];
        }
    }
    
    str[i] = '\0';
    va_end(args);
    return i;
}