#include "memory.h"
#include "kernel.h"
#include "string.h"
#include "error.h"

#include <stdint.h>

// Memory pool structure
struct memory_pool {
    void* buffer;
    size_t block_size;
    size_t num_blocks;
    int* free_blocks; // Array to track free blocks
    size_t free_count;
};

// Enhanced memory management functions
size_t get_memory_usage(void) {
    // In a real implementation, this would return actual memory usage
    // For now, we'll return a placeholder value
    return 0;
}

size_t get_free_memory(void) {
    // In a real implementation, this would return free memory
    // For now, we'll return a placeholder value
    return 0;
}

// Memory pool functions
memory_pool_t* create_memory_pool(size_t block_size, size_t num_blocks) {
    (void)block_size;
    (void)num_blocks;
    // In a real implementation, this would create a memory pool
    // For now, we'll return NULL to indicate failure
    serial_write("MEMORY: create_memory_pool called (stub)\n");
    return NULL;
}

void destroy_memory_pool(memory_pool_t* pool) {
    // In a real implementation, this would destroy a memory pool
    // For now, we'll just do nothing
    serial_write("MEMORY: destroy_memory_pool called (stub)\n");
    (void)pool; // Suppress unused parameter warning
}

void* pool_malloc(memory_pool_t* pool, size_t size) {
    // In a real implementation, this would allocate from a memory pool
    // For now, we'll return NULL to indicate failure
    serial_write("MEMORY: pool_malloc called (stub)\n");
    (void)pool; // Suppress unused parameter warning
    (void)size; // Suppress unused parameter warning
    return NULL;
}

void pool_free(memory_pool_t* pool, void* ptr) {
    // In a real implementation, this would free memory to a memory pool
    // For now, we'll just do nothing
    serial_write("MEMORY: pool_free called (stub)\n");
    (void)pool; // Suppress unused parameter warning
    (void)ptr;  // Suppress unused parameter warning
}

// Initialize memory management system
void memory_init(void) {
    serial_write("MEMORY: Initializing memory management system\n");
    memory_init_heap();
    serial_write("MEMORY: Memory management system initialized\n");
}

// Stack guard: fill a known region of the stack with a canary pattern.
// Call once early in kernel_main before the stack grows too far.
#define STACK_CANARY_PATTERN 0xDEADBEEF
#define STACK_CANARY_SIZE    256

static uint32_t* stack_canary_base;
static size_t    stack_canary_words;

void memory_init_stack_guard(uintptr_t base, size_t size) {
    if (size < STACK_CANARY_SIZE * sizeof(uint32_t)) {
        size = STACK_CANARY_SIZE * sizeof(uint32_t);
    }
    stack_canary_words = size / sizeof(uint32_t);
    stack_canary_base  = (uint32_t*)base;

    for (size_t i = 0; i < stack_canary_words; i++) {
        stack_canary_base[i] = STACK_CANARY_PATTERN;
    }
    serial_write("MEMORY: stack canary initialized\n");
}

// Check how many canary words are still intact; returns number of remaining intact words.
size_t memory_check_stack_guard(void) {
    size_t intact = 0;
    for (size_t i = 0; i < stack_canary_words; i++) {
        if (stack_canary_base[i] == STACK_CANARY_PATTERN) {
            intact++;
        } else {
            break;
        }
    }
    return intact;
}

// Report remaining stack margin in bytes; logs a warning if it falls below threshold.
size_t memory_get_stack_margin(void) {
    if (stack_canary_words == 0) {
        return 0;
    }
    size_t intact = memory_check_stack_guard();
    size_t margin = intact * sizeof(uint32_t);
    if (margin < 256) {
        LOG_WARN("stack margin low: %zu bytes", margin);
    }
    return margin;
}