/*
 * kernel.c - Main kernel implementation
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "string.h"
#include "network.h"
#include "c_model_interface.h"
#include "memory.h"
#include "error.h"

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

// Function to write to I/O ports
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Function to read from I/O ports
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Function to create VGA color entry
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

// Function to create VGA entry
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

// Initialize serial port
void serial_init(void) {
    // Disable interrupts
    outb(0x3F8 + 1, 0x00);
    // Enable DLAB (set baud rate divisor)
    outb(0x3F8 + 3, 0x80);
    // Set divisor to 1 (lo byte) 115200 baud
    outb(0x3F8, 0x01);
    //                  (hi byte)
    outb(0x3F8 + 1, 0x00);
    // 8 bits, no parity, one stop bit
    outb(0x3F8 + 3, 0x03);
    // Enable FIFO, clear them, with 14-byte threshold
    outb(0x3F8 + 2, 0xC7);
    // IRQs enabled, RTS/DSR set
    outb(0x3F8 + 4, 0x0B);
}

// Initialize VGA hardware
void vga_init(void) {
    serial_write("VGA: Initializing VGA hardware\n");
    
    // Simple approach - just clear the screen
    terminal_buffer = (uint16_t*) 0xB8000;
    
    // Clear screen
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    
    serial_write("VGA: VGA hardware initialized\n");
}

// Update cursor position
static void update_cursor(int row, int col) {
    unsigned short position = (row * VGA_WIDTH) + col;
    
    // Cursor LOW port to VGA INDEX REGISTER
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    
    // Cursor HIGH port to VGA INDEX REGISTER
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

// Write to serial port
void serial_write(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        // Wait for serial port to be ready
        while (!(inb(0x3F8 + 5) & 0x20));
        outb(0x3F8, data[i]);
    }
}

// Initialize terminal
void terminal_init(void) {
    serial_write("TERMINAL: Initializing terminal\n");
    
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
    
    // Update cursor position
    update_cursor(terminal_row, terminal_column);
    
    serial_write("TERMINAL: Terminal initialized\n");
}

// Put a character at the current cursor position
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }
        update_cursor(terminal_row, terminal_column);
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
    
    update_cursor(terminal_row, terminal_column);
}

// Write data to terminal
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

// Write a null-terminated string to terminal
void terminal_writestring(const char* data) {
    // Fallback to serial output if VGA fails
    if (!terminal_buffer) {
        serial_write(data);
        return;
    }
    
    terminal_write(data, strlen(data));
}

// Boot-time sanity checks. Returns 0 on success, -1 on failure.
static int run_self_tests(void) {
    int failed = 0;

    // Allocator sanity check
    char* p = (char*)malloc(64);
    if (!p) {
        serial_write("KERNEL: Self-test failed: malloc\n");
        failed++;
    } else {
        for (int i = 0; i < 64; i++) {
            p[i] = (char)(i & 0xFF);
        }
        int ok = 1;
        for (int i = 0; i < 64; i++) {
            if (p[i] != (char)(i & 0xFF)) {
                ok = 0;
                break;
            }
        }
        if (!ok) {
            serial_write("KERNEL: Self-test failed: memory pattern\n");
            failed++;
        }
        free(p);
    }

    // Health endpoint sanity check
    http_request_t* request = (http_request_t*)malloc(sizeof(http_request_t));
    http_response_t* response = (http_response_t*)malloc(sizeof(http_response_t));
    if (!request || !response) {
        serial_write("KERNEL: Self-test failed: request/response allocation\n");
        failed++;
    } else {
        strncpy(request->path, "/health", sizeof(request->path));
        strncpy(request->method, "GET", sizeof(request->method));
        if (route_request(request, response) != 0 || response->status_code != 200) {
            serial_write("KERNEL: Self-test failed: /health endpoint\n");
            failed++;
        }
        free(request);
        free(response);
    }

    // Embedded model sanity check
    embedded_model_info_t model_info = c_load_embedded_model();
    if (!model_info.is_valid) {
        serial_write("KERNEL: Self-test failed: embedded model load\n");
        failed++;
    }

    return failed == 0 ? 0 : -1;
}

// Kernel main function
void kernel_main(void) {
    serial_init();
    serial_write("KERNEL: CLLM unikernel booting\n");

    memory_init();
    serial_write("KERNEL: Memory initialized\n");

    vga_init();
    terminal_init();
    terminal_writestring("CLLM unikernel\n");

    if (network_init() == 0) {
        serial_write("KERNEL: Network initialized\n");
    } else {
        serial_write("KERNEL: Network initialization failed; continuing without networking\n");
    }

    if (run_self_tests() != 0) {
        serial_write("KERNEL: Self-tests failed; halting\n");
        terminal_writestring("Self-tests failed; halting\n");
        while (1) {
            __asm__("hlt");
        }
    }
    serial_write("KERNEL: Self-tests passed\n");

    serial_write("KERNEL: Entering network loop\n");
    network_loop();
}
