/*
 * test_memory.c - Simple test program for memory management functions
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/c_bindings.h"

int main() {
    printf("Testing memory management functions...\\n");
    
    // Test basic malloc/free
    void* ptr = zig_malloc(1024);
    if (ptr == NULL) {
        printf("Failed to allocate memory\\n");
        return 1;
    }
    
    printf("Allocated 1024 bytes at %p\\n", ptr);
    
    // Test memory usage
    size_t usage = zig_get_memory_usage();
    printf("Current memory usage: %zu bytes\\n", usage);
    
    // Free the memory
    zig_free(ptr);
    printf("Freed memory\\n");
    
    // Test realloc
    ptr = zig_malloc(512);
    if (ptr == NULL) {
        printf("Failed to allocate memory\\n");
        return 1;
    }
    
    printf("Allocated 512 bytes at %p\\n", ptr);
    
    usage = zig_get_memory_usage();
    printf("Current memory usage: %zu bytes\\n", usage);
    
    // Test realloc (currently just allocates new memory)
    void* new_ptr = zig_realloc(ptr, 1024);
    if (new_ptr == NULL) {
        printf("Failed to reallocate memory\\n");
        zig_free(ptr);
        return 1;
    }
    
    printf("Reallocated to 1024 bytes at %p\\n", new_ptr);
    
    usage = zig_get_memory_usage();
    printf("Current memory usage: %zu bytes\\n", usage);
    
    zig_free(new_ptr);
    printf("Freed reallocated memory\\n");
    
    // Test memory pool
    printf("\\nTesting memory pool...\\n");
    zig_memory_pool_t* pool = zig_memory_pool_create(256, 10);
    if (pool == NULL) {
        printf("Failed to create memory pool\\n");
        return 1;
    }
    
    printf("Created memory pool\\n");
    
    void* pool_ptr = zig_memory_pool_alloc(pool, 128);
    if (pool_ptr == NULL) {
        printf("Failed to allocate from pool\\n");
        zig_memory_pool_destroy(pool);
        return 1;
    }
    
    printf("Allocated 128 bytes from pool at %p\\n", pool_ptr);
    
    zig_memory_pool_free(pool, pool_ptr);
    printf("Freed pool memory\\n");
    
    zig_memory_pool_destroy(pool);
    printf("Destroyed memory pool\\n");
    
    printf("All memory tests completed successfully!\\n");
    return 0;
}