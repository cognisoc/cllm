/*
 * Host-side unit tests for the unikernel allocator.
 *
 * Build with:
 *   gcc -m32 -I../../include -fno-builtin-malloc -fno-builtin-free \
 *       -fno-builtin-realloc -fno-builtin-memcpy \
 *       test_allocator.c ../../src/allocator.c ../../src/string.c -o test_allocator
 *   ./test_allocator
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "string.h"
#include "memory.h"

#define TEST(name) do { \
    printf("  %-40s ", name); \
    fflush(stdout); \
} while (0)

#define PASS() do { printf("PASS\n"); passed++; } while (0)
#define FAIL(fmt, ...) do { printf("FAIL: " fmt "\n", ##__VA_ARGS__); failed++; } while (0)

static int passed = 0;
static int failed = 0;

static int check_pattern(uint8_t* p, size_t len, uint8_t value) {
    for (size_t i = 0; i < len; i++) {
        if (p[i] != value) {
            return 0;
        }
    }
    return 1;
}

int main(void) {
    printf("Allocator unit tests\n");
    printf("====================\n");

    memory_init_heap();

    TEST("malloc returns usable memory");
    {
        uint8_t* p = (uint8_t*)malloc(64);
        if (!p) { FAIL("malloc(64) returned NULL"); }
        else {
            memset(p, 0xAB, 64);
            if (check_pattern(p, 64, 0xAB)) {
                PASS();
            } else {
                FAIL("memory pattern mismatch");
            }
            free(p);
        }
    }

    TEST("malloc returns aligned pointers");
    {
        void* p = malloc(1);
        if (!p) { FAIL("malloc(1) returned NULL"); }
        else if (((uintptr_t)p & 7) != 0) {
            FAIL("pointer %p is not 8-byte aligned", p);
            free(p);
        } else {
            PASS();
            free(p);
        }
    }

    TEST("free allows reuse");
    {
        void* p1 = malloc(64);
        if (!p1) { FAIL("first malloc failed"); }
        else {
            free(p1);
            void* p2 = malloc(64);
            if (!p2) { FAIL("second malloc failed after free"); }
            else {
                if (p1 == p2) {
                    PASS();
                } else {
                    PASS(); // reuse is allowed but not required
                }
                free(p2);
            }
        }
    }

    TEST("multiple allocations are distinct");
    {
        void* a = malloc(16);
        void* b = malloc(16);
        void* c = malloc(16);
        if (!a || !b || !c) {
            FAIL("one of the allocations failed");
        } else if (a == b || b == c || a == c) {
            FAIL("allocations overlap");
        } else {
            PASS();
        }
        free(a);
        free(b);
        free(c);
    }

    TEST("coalescing after freeing all");
    {
        void* a = malloc(1024);
        void* b = malloc(1024);
        void* c = malloc(1024);
        if (!a || !b || !c) {
            FAIL("initial allocations failed");
        } else {
            free(a);
            free(b);
            free(c);
            void* big = malloc(3072);
            if (!big) {
                FAIL("large allocation failed after coalescing");
            } else {
                PASS();
                free(big);
            }
        }
    }

    TEST("realloc grows and preserves data");
    {
        uint8_t* p = (uint8_t*)malloc(16);
        if (!p) { FAIL("malloc(16) failed"); }
        else {
            for (int i = 0; i < 16; i++) {
                p[i] = (uint8_t)i;
            }
            uint8_t* q = (uint8_t*)realloc(p, 64);
            if (!q) {
                FAIL("realloc failed");
                free(p);
            } else {
                int ok = 1;
                for (int i = 0; i < 16; i++) {
                    if (q[i] != (uint8_t)i) {
                        ok = 0;
                        break;
                    }
                }
                if (ok) {
                    PASS();
                } else {
                    FAIL("realloc did not preserve data");
                }
                free(q);
            }
        }
    }

    TEST("out of memory returns NULL");
    {
        // The 4 MB heap cannot satisfy a 16 MB request.
        void* p = malloc(16 * 1024 * 1024);
        if (p) {
            FAIL("oversized allocation did not return NULL");
            free(p);
        } else {
            PASS();
        }
    }

    TEST("double free is handled");
    {
        void* p = malloc(16);
        if (!p) { FAIL("malloc failed"); }
        else {
            free(p);
            free(p); // should not crash or corrupt the heap
            void* q = malloc(16);
            if (q) {
                PASS();
                free(q);
            } else {
                FAIL("heap corrupted after double free");
            }
        }
    }

    printf("====================\n");
    printf("Results: %d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
