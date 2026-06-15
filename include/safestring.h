#ifndef SAFE_STRING_H
#define SAFE_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Length-bounded string helpers that always null-terminate the destination.
size_t safe_strcpy(char* dst, size_t dst_size, const char* src);
size_t safe_strcat(char* dst, size_t dst_size, const char* src);
size_t safe_strncpy(char* dst, size_t dst_size, const char* src, size_t n);

// Bounds-checked formatted append. Writes into dst starting at offset and updates
// offset. Returns number of bytes appended.
int safe_snprintf_append(char* dst, size_t dst_size, size_t* offset,
                         const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif
