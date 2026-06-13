/*
 * string.c - Simple string utilities implementation
 */

#include "string.h"
#include <stdarg.h>
#include <stdint.h>

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

// Simple implementation of memset
void* memset(void* s, int c, size_t n) {
    unsigned char* p = (unsigned char*)s;
    
    for (size_t i = 0; i < n; i++) {
        p[i] = (unsigned char)c;
    }
    
    return s;
}

// Simple implementation of memcpy
void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

// Simple implementation of memcmp
int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* a = (const unsigned char*)s1;
    const unsigned char* b = (const unsigned char*)s2;
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) return (int)a[i] - (int)b[i];
    }
    return 0;
}

// Simple implementation of strcmp
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

// Helpers for snprintf
static void append_char(char* dst, size_t size, size_t* off, char c) {
    if (*off + 1 < size) {
        dst[*off] = c;
    }
    (*off)++;
}

static void append_str(char* dst, size_t size, size_t* off, const char* s) {
    for (size_t i = 0; s[i] != '\0'; i++) {
        append_char(dst, size, off, s[i]);
    }
}

static void append_uint(char* dst, size_t size, size_t* off, unsigned long v, int base, int is_upper) {
    char buf[32];
    const char* digits = is_upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;
    if (v == 0) {
        buf[i++] = '0';
    } else {
        while (v && i < (int)sizeof(buf)) {
            buf[i++] = digits[v % (unsigned)base];
            v /= (unsigned)base;
        }
    }
    while (i--) {
        append_char(dst, size, off, buf[i]);
    }
}

int vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    size_t off = 0;
    for (size_t i = 0; format[i] != '\0'; i++) {
        if (format[i] != '%') {
            append_char(str, size, &off, format[i]);
            continue;
        }
        i++;
        if (format[i] == 's') {
            const char* s = va_arg(ap, const char*);
            if (!s) s = "(null)";
            append_str(str, size, &off, s);
        } else if (format[i] == 'd' || format[i] == 'i') {
            int v = va_arg(ap, int);
            if (v < 0) { append_char(str, size, &off, '-'); append_uint(str, size, &off, (unsigned long)(-(long)v), 10, 0); }
            else { append_uint(str, size, &off, (unsigned long)v, 10, 0); }
        } else if (format[i] == 'u') {
            unsigned int v = va_arg(ap, unsigned int);
            append_uint(str, size, &off, v, 10, 0);
        } else if (format[i] == 'x') {
            unsigned int v = va_arg(ap, unsigned int);
            append_uint(str, size, &off, v, 16, 0);
        } else if (format[i] == 'X') {
            unsigned int v = va_arg(ap, unsigned int);
            append_uint(str, size, &off, v, 16, 1);
        } else if (format[i] == 'p') {
            unsigned long v = (unsigned long)(uintptr_t)va_arg(ap, void*);
            append_str(str, size, &off, "0x");
            append_uint(str, size, &off, v, 16, 0);
        } else if (format[i] == 'z' && format[i+1] == 'u') {
            i++;
            size_t v = va_arg(ap, size_t);
            append_uint(str, size, &off, (unsigned long)v, 10, 0);
        } else if (format[i] == '%') {
            append_char(str, size, &off, '%');
        } else {
            // Unsupported specifier; emit literally for visibility
            append_char(str, size, &off, '%');
            append_char(str, size, &off, format[i]);
        }
    }
    // Null-terminate if space
    if (size > 0) {
        size_t term = off < size ? off : size - 1;
        str[term] = '\0';
    }
    return (int)off;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    int n = vsnprintf(str, size, format, ap);
    va_end(ap);
    return n;
}
