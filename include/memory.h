#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

// Enhanced memory management functions
void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);

// Memory statistics
size_t get_memory_usage(void);
size_t get_free_memory(void);

// Memory pool functions
typedef struct memory_pool memory_pool_t;

memory_pool_t* create_memory_pool(size_t block_size, size_t num_blocks);
void destroy_memory_pool(memory_pool_t* pool);
void* pool_malloc(memory_pool_t* pool, size_t size);
void pool_free(memory_pool_t* pool, void* ptr);

// Initialize memory management system
void memory_init(void);

#endif // MEMORY_H