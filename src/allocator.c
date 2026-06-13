/*
 * allocator.c - Simple heap allocator for the unikernel
 *
 * Provides malloc/free/realloc backed by a statically allocated 4 MB arena.
 * The allocator uses a first-fit strategy with free-list coalescing.
 */

#include "string.h"
#include <stddef.h>

#define HEAP_SIZE (4 * 1024 * 1024) // 4MB heap for LLM serving
static char heap[HEAP_SIZE];
static int heap_initialized = 0;

// Memory block header for tracking allocations
typedef struct mem_block {
    size_t size;            // payload size, excluding this header
    int is_free;
    struct mem_block* next; // next block in the free list
} mem_block_t;

// 8-byte aligned header size so every payload is 8-byte aligned.
#define HEADER_SIZE ((sizeof(mem_block_t) + 7) & ~7)

// Head of the free list (kept sorted by address)
static mem_block_t* free_list_head = NULL;

// Initialize the heap as one large free block.
void memory_init_heap(void) {
    if (heap_initialized) {
        return;
    }

    free_list_head = (mem_block_t*)heap;
    free_list_head->size = HEAP_SIZE - HEADER_SIZE;
    free_list_head->is_free = 1;
    free_list_head->next = NULL;
    heap_initialized = 1;
}

// Ensure the heap is initialized before any allocation.
static void heap_ensure_initialized(void) {
    if (!heap_initialized) {
        memory_init_heap();
    }
}

// Simple malloc implementation with first-fit and splitting.
void* malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    heap_ensure_initialized();

    // Align size to 8 bytes for better performance and to keep headers aligned.
    size = (size + 7) & ~7;
    if (size == 0) {
        size = 8;
    }

    mem_block_t* current = free_list_head;
    mem_block_t* prev = NULL;

    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            size_t total_size = size + HEADER_SIZE;

            // Remove the block from the free list (it will be allocated).
            if (prev != NULL) {
                prev->next = current->next;
            } else {
                free_list_head = current->next;
            }

            // Split if enough room remains for another allocation.
            if (current->size >= total_size + HEADER_SIZE + 8) {
                mem_block_t* new_block = (mem_block_t*)((char*)current + total_size);
                new_block->size = current->size - total_size;
                new_block->is_free = 1;
                new_block->next = NULL;

                // Insert the remainder back into the free list in address order.
                mem_block_t** pos = &free_list_head;
                while (*pos != NULL && *pos < new_block) {
                    pos = &(*pos)->next;
                }
                new_block->next = *pos;
                *pos = new_block;

                current->size = size;
            }

            current->is_free = 0;
            current->next = NULL;
            return (char*)current + HEADER_SIZE;
        }

        prev = current;
        current = current->next;
    }

    return NULL; // Out of memory
}

// Mark a block as free, return it to the free list, and coalesce neighbors.
void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    mem_block_t* block = (mem_block_t*)((char*)ptr - HEADER_SIZE);
    if (block->is_free) {
        // Double-free guard.
        return;
    }

    block->is_free = 1;
    block->next = NULL;

    // Insert into the free list sorted by address.
    mem_block_t** pos = &free_list_head;
    while (*pos != NULL && *pos < block) {
        pos = &(*pos)->next;
    }
    block->next = *pos;
    *pos = block;

    // Coalesce adjacent free blocks.
    mem_block_t* curr = free_list_head;
    while (curr != NULL && curr->next != NULL) {
        char* curr_end = (char*)curr + HEADER_SIZE + curr->size;
        if (curr_end == (char*)curr->next) {
            curr->size += HEADER_SIZE + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

// Simple realloc implementation.
void* realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    mem_block_t* block = (mem_block_t*)((char*)ptr - HEADER_SIZE);
    size_t current_size = block->size;

    if (size <= current_size) {
        block->size = size;
        return ptr;
    }

    void* new_ptr = malloc(size);
    if (new_ptr != NULL) {
        memcpy(new_ptr, ptr, current_size);
        free(ptr);
    }

    return new_ptr;
}
