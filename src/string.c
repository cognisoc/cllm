/*
 * string.c - Simple string utilities implementation
 */

#include "string.h"
#include "kernel.h"
#include <stdarg.h>

// Enhanced memory allocator with free list
#define HEAP_SIZE (4 * 1024 * 1024) // 4MB heap for LLM serving
static char heap[HEAP_SIZE];
static size_t heap_offset = 0;

// Memory block header for tracking allocations
typedef struct mem_block {
    size_t size;
    int is_free;
    struct mem_block* next;
} mem_block_t;

// Head of the free list
static mem_block_t* free_list_head = NULL;

// Simple malloc implementation
void* malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    // Align size to 8 bytes for better performance
    size = (size + 7) & ~7;
    
    // Add header size
    size_t total_size = size + sizeof(mem_block_t);
    
    // First fit algorithm with free list
    mem_block_t* current = free_list_head;
    mem_block_t* prev = NULL;
    
    // Search free list for suitable block
    while (current != NULL) {
        if (current->is_free && current->size >= total_size) {
            // Found a suitable block
            if (current->size > total_size + sizeof(mem_block_t) + 8) {
                // Split the block if there's enough space for another block
                mem_block_t* new_block = (mem_block_t*)((char*)current + total_size);
                new_block->size = current->size - total_size;
                new_block->is_free = 1;
                new_block->next = current->next;
                
                current->size = size; // Store requested size, not total size
                current->is_free = 0;
                current->next = new_block;
            } else {
                // Use the whole block
                current->size = size; // Store requested size
                current->is_free = 0;
                
                // Remove from free list
                if (prev != NULL) {
                    prev->next = current->next;
                } else {
                    free_list_head = current->next;
                }
            }
            
            return (char*)current + sizeof(mem_block_t);
        }
        
        prev = current;
        current = current->next;
    }
    
    // No suitable block found in free list, try extending heap
    if (heap_offset + total_size > HEAP_SIZE) {
        return NULL; // Out of memory
    }
    
    // Allocate new block at the end of heap
    mem_block_t* new_block = (mem_block_t*)(heap + heap_offset);
    new_block->size = size;
    new_block->is_free = 0;
    new_block->next = NULL;
    
    heap_offset += total_size;
    return (char*)new_block + sizeof(mem_block_t);
}

// Simple free implementation
void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
    // Get the block header
    
    // In this simple allocator, we don't actually free memory
    // A more sophisticated allocator would track allocations
    // For now, we'll just log that free was called
    serial_write("FREE: Called with ptr=");
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%p", ptr);
    serial_write(buffer);
    serial_write("\n");
    
    // In a real implementation, we would add the block back to the free list
    // or mark it as free for reuse
    // block->is_free = 1;
    // 
    // Add to free list
    // block->next = free_list_head;
    // free_list_head = block;
    // 
    // Coalesce adjacent free blocks
    // mem_block_t* current = free_list_head;
    // while (current != NULL && current->next != NULL) {
    //     char* current_end = (char*)current + sizeof(mem_block_t) + current->size;
    //     mem_block_t* next_block = current->next;
    //     
    //     if (current_end == (char*)next_block && next_block->is_free) {
    //         // Merge with next block
    //         current->size += sizeof(mem_block_t) + next_block->size;
    //         current->next = next_block->next;
    //     } else {
    //         current = current->next;
    //     }
    // }
}

// Simple realloc implementation
void* realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }
    
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    // Get current block size
    mem_block_t* block = (mem_block_t*)((char*)ptr - sizeof(mem_block_t));
    size_t current_size = block->size;
    
    if (size <= current_size) {
        // Shrink or same size, just return the same pointer
        block->size = size;
        return ptr;
    }
    
    // Need to grow, allocate new block and copy data
    void* new_ptr = malloc(size);
    if (new_ptr != NULL) {
        // Copy data
        char* src = (char*)ptr;
        char* dst = (char*)new_ptr;
        for (size_t i = 0; i < current_size && i < size; i++) {
            dst[i] = src[i];
        }
        free(ptr);
    }
    
    return new_ptr;
}

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
