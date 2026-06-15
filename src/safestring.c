/*
 * safestring.c - Bounds-checked string helpers.
 */

#include "safestring.h"
#include "string.h"
#include <stdarg.h>

size_t safe_strcpy(char* dst, size_t dst_size, const char* src) {
    if (dst_size == 0 || !dst || !src) return 0;
    size_t i;
    for (i = 0; i < dst_size - 1 && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    return i;
}

size_t safe_strncpy(char* dst, size_t dst_size, const char* src, size_t n) {
    if (dst_size == 0 || !dst || !src) return 0;
    size_t i;
    for (i = 0; i < dst_size - 1 && i < n && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    return i;
}

size_t safe_strcat(char* dst, size_t dst_size, const char* src) {
    if (dst_size == 0 || !dst || !src) return 0;
    size_t dst_len = 0;
    while (dst_len < dst_size && dst[dst_len] != '\0') {
        dst_len++;
    }
    if (dst_len >= dst_size) return 0;
    size_t copied = safe_strcpy(dst + dst_len, dst_size - dst_len, src);
    return dst_len + copied;
}

int safe_snprintf_append(char* dst, size_t dst_size, size_t* offset,
                         const char* format, ...) {
    if (!dst || !offset || dst_size == 0 || *offset >= dst_size) return -1;

    va_list ap;
    va_start(ap, format);
    int n = vsnprintf(dst + *offset, dst_size - *offset, format, ap);
    va_end(ap);

    if (n < 0) return n;
    if ((size_t)n >= dst_size - *offset) {
        *offset = dst_size - 1;
        dst[*offset] = '\0';
        return (int)(dst_size - 1 - *offset);
    }
    *offset += (size_t)n;
    return n;
}
