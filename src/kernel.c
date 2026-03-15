/*
 * kernel.c - Main kernel implementation
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "string.h"
#include "network.h"

// VGA text buffer
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

// VGA color constants
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

// Function to create VGA color entry
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

// Function to create VGA entry
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

// Initialize terminal
void terminal_init(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

// Put a character at the current cursor position
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }
        return;
    }
    
    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    terminal_buffer[index] = vga_entry(c, terminal_color);
    
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }
    }
}

// Write data to terminal
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

// Write a null-terminated string to terminal
void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

// Test our HTTP API handlers
void test_api_handlers(void) {
    terminal_writestring("Testing API handlers...\n");
    
    // Test health check endpoint
    http_request_t request;
    http_response_t response;
    
    // Mock a health check request
    strncpy(request.path, "/health", sizeof(request.path));
    int result = route_request(&request, &response);
    
    if (result == 0 && response.status_code == 200) {
        terminal_writestring("Health check handler works\n");
    } else {
        terminal_writestring("Health check handler failed\n");
    }
    
    // Test model loading endpoint
    strncpy(request.path, "/models/load", sizeof(request.path));
    result = route_request(&request, &response);
    
    if (result == 0 && response.status_code == 200) {
        terminal_writestring("Model loading handler works\n");
    } else {
        terminal_writestring("Model loading handler failed\n");
    }
    
    // Test completion endpoint
    strncpy(request.path, "/completion", sizeof(request.path));
    result = route_request(&request, &response);
    
    if (result == 0 && response.status_code == 200) {
        terminal_writestring("Completion handler works\n");
    } else {
        terminal_writestring("Completion handler failed\n");
    }
    
    // Test unknown endpoint
    strncpy(request.path, "/unknown", sizeof(request.path));
    result = route_request(&request, &response);
    
    if (result == 0 && response.status_code == 404) {
        terminal_writestring("Unknown endpoint handler works\n");
    } else {
        terminal_writestring("Unknown endpoint handler failed\n");
    }
}

// Kernel main function
void kernel_main(void) {
    terminal_init();
    terminal_writestring("Hello from our custom unikernel!\n");
    terminal_writestring("This is a minimal kernel implementation.\n");
    terminal_writestring("\n");
    
    // Test our API handlers
    test_api_handlers();
    
    terminal_writestring("\nAll tests completed.\n");
    
    // Halt the CPU
    while (1) {
        __asm__("hlt");
    }
}