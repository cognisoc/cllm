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
#include "llm.h"
#include "cuda_interface.h"
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

// Test our HTTP API handlers
void test_api_handlers(void) {
    terminal_writestring("Testing API handlers...\n");
    serial_write("KERNEL: Testing API handlers...\n");
    
    // Test malloc function
    serial_write("KERNEL: Testing malloc...\n");
    char* test_malloc = (char*)malloc(32);
    if (test_malloc) {
        serial_write("KERNEL: malloc succeeded\n");
        free(test_malloc);
        serial_write("KERNEL: free succeeded\n");
    } else {
        serial_write("KERNEL: malloc failed\n");
    }
    
    // Test strncpy function directly
    serial_write("KERNEL: Testing strncpy...\n");
    char test_buffer[32];
    strncpy(test_buffer, "/health", sizeof(test_buffer));
    serial_write("KERNEL: strncpy completed\n");
    
    // Test strncpy with allocated memory
    serial_write("KERNEL: Testing strncpy with allocated memory...\n");
    char* allocated_buffer = (char*)malloc(32);
    if (allocated_buffer) {
        serial_write("KERNEL: Allocated buffer\n");
        strncpy(allocated_buffer, "/health", 32);
        serial_write("KERNEL: strncpy with allocated memory completed\n");
        free(allocated_buffer);
        serial_write("KERNEL: Freed allocated buffer\n");
    } else {
        serial_write("KERNEL: Failed to allocate buffer\n");
    }
    
    // Test health check endpoint
    serial_write("KERNEL: Allocating request and response...\n");
    http_request_t* request = (http_request_t*)malloc(sizeof(http_request_t));
    http_response_t* response = (http_response_t*)malloc(sizeof(http_response_t));
    
    if (!request || !response) {
        serial_write("KERNEL: Failed to allocate request or response\n");
        terminal_writestring("Failed to allocate request or response\n");
        return;
    }
    
    serial_write("KERNEL: Request and response allocated\n");
    serial_write("KERNEL: Request address check\n");
    serial_write("KERNEL: Response address check\n");
    
    // Mock a health check request
    serial_write("KERNEL: Setting up request...\n");
    serial_write("KERNEL: About to call strncpy...\n");
    serial_write("KERNEL: Request path address check\n");
    
    // Let's try a different approach - copy character by character
    serial_write("KERNEL: Copying character by character\n");
    const char* source = "/health";
    for (int i = 0; i < 8 && source[i] != '\0'; i++) { // 8 is length of "/health" + 1 for null terminator
        request->path[i] = source[i];
    }
    request->path[7] = '\0'; // Null terminate
    serial_write("KERNEL: Character by character copy completed\n");
    
    serial_write("KERNEL: Request path set\n");
    serial_write("KERNEL: Calling route_request for /health...\n");
    int route_result = route_request(request, response);
    serial_write("KERNEL: route_request returned with result: ");
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d", route_result);
    serial_write(buffer);
    serial_write("\n");
    terminal_writestring("Health check handler tested\n");
    
    // Clean up
    free(request);
    free(response);
    
    serial_write("KERNEL: API handler tests completed\n");
}

// Test LLM functionality
void test_llm(void) {
    serial_write("KERNEL: Testing LLM functionality...\n");
    terminal_writestring("Testing LLM functionality...\n");
    
    // Get embedded model
    serial_write("KERNEL: Loading embedded model...\n");
    embedded_model_info_t model_info = c_load_embedded_model();
    if (!model_info.is_valid) {
        serial_write("KERNEL: Failed to load embedded model\n");
        terminal_writestring("Failed to load embedded model\n");
        return;
    }
    
    serial_write("KERNEL: Embedded model loaded successfully\n");
    terminal_writestring("Embedded model loaded successfully\n");
    
    // Initialize LLM context
    serial_write("KERNEL: Initializing LLM context...\n");
    llm_params_t params = llm_default_params();
    llm_context_t* ctx = llm_init(model_info.data, model_info.size, &params);
    if (!ctx) {
        serial_write("KERNEL: Failed to initialize LLM context\n");
        terminal_writestring("Failed to initialize LLM context\n");
        return;
    }
    
    serial_write("KERNEL: LLM context initialized\n");
    terminal_writestring("LLM context initialized\n");
    
    // Test tokenization
    serial_write("KERNEL: Testing tokenization...\n");
    const char* test_text = "Hello, world!";
    int tokens[64];
    int token_count = llm_tokenize(ctx, test_text, tokens, 64);
    if (token_count > 0) {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "Tokenized '%s' into %d tokens\n", test_text, token_count);
        terminal_writestring(buffer);
    } else {
        terminal_writestring("Failed to tokenize text\n");
    }
    
    // Test evaluation
    serial_write("KERNEL: Testing evaluation...\n");
    if (llm_eval(ctx, tokens, token_count, 1) == 0) {
        terminal_writestring("Evaluation completed\n");
    } else {
        terminal_writestring("Evaluation failed\n");
    }
    
    // Test sampling
    serial_write("KERNEL: Testing sampling...\n");
    int next_token;
    if (llm_sample(ctx, &next_token) == 0) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Sampled next token: %d\n", next_token);
        terminal_writestring(buffer);
    } else {
        terminal_writestring("Sampling failed\n");
    }
    
    // Clean up
    serial_write("KERNEL: Cleaning up...\n");
    llm_free(ctx);
    serial_write("KERNEL: LLM test completed\n");
    terminal_writestring("LLM test completed\n");
}

// Test CUDA functionality
void test_cuda(void) {
    serial_write("KERNEL: Testing CUDA functionality...\n");
    terminal_writestring("Testing CUDA functionality...\n");
    
    // Check if CUDA is available
    serial_write("KERNEL: Checking if CUDA is available...\n");
    if (cuda_uk_is_available()) {
        terminal_writestring("CUDA is available\n");
        
        // Get device count
        int device_count = cuda_uk_get_device_count();
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Found %d CUDA devices\n", device_count);
        terminal_writestring(buffer);
        
        // Initialize CUDA
        cuda_unikernel_context_t cuda_ctx;
        cuda_uk_error_t result = cuda_uk_init(&cuda_ctx, 0);
        if (result == CUDA_UK_SUCCESS) {
            terminal_writestring("CUDA initialized successfully\n");
            
            // Test memory allocation
            void* gpu_mem = cuda_uk_malloc(&cuda_ctx, 1024);
            if (gpu_mem) {
                terminal_writestring("GPU memory allocated\n");
                cuda_uk_free(&cuda_ctx, gpu_mem);
                terminal_writestring("GPU memory freed\n");
            } else {
                terminal_writestring("Failed to allocate GPU memory\n");
            }
        } else {
            terminal_writestring("Failed to initialize CUDA\n");
        }
    } else {
        terminal_writestring("CUDA is not available\n");
    }
    
    serial_write("KERNEL: CUDA test completed\n");
    terminal_writestring("CUDA test completed\n");
}

// Test error handling functionality
void test_error_handling(void) {
    serial_write("KERNEL: Testing error handling functionality...\n");
    terminal_writestring("Testing error handling functionality...\n");
    
    // Test basic logging
    serial_write("KERNEL: Testing basic logging...\n");
    LOG_DEBUG("This is a debug message");
    LOG_INFO("This is an info message");
    LOG_WARN("This is a warning message");
    LOG_ERROR("This is an error message");
    
    // Test error logging
    serial_write("KERNEL: Testing error logging...\n");
    LOG_ERR(ERR_OUT_OF_MEMORY, "Failed to allocate memory for buffer");
    LOG_ERR(ERR_INVALID_PARAMETER, "Invalid parameter passed to function");
    
    // Test conditional error handling
    serial_write("KERNEL: Testing conditional error handling...\n");
    int test_value = -1;
    if (test_value < 0) {
        LOG_ERR(ERR_INVALID_PARAMETER, "Test value must be positive");
        // In a real implementation, we would return an error code
        // But since this is a void function, we'll just log and continue
    }
    
    serial_write("KERNEL: Error handling test completed\n");
    terminal_writestring("Error handling test completed\n");
}

// Kernel main function
void kernel_main(void) {
    // Initialize serial port
    serial_init();
    serial_write("KERNEL: Serial port initialized\n");
    
    // Initialize memory management system
    memory_init();
    serial_write("KERNEL: Memory management system initialized\n");
    
    // Initialize VGA hardware
    vga_init();
    serial_write("KERNEL: VGA initialized\n");
    
    // Initialize terminal
    terminal_init();
    terminal_writestring("Hello from our custom unikernel!\n");
    terminal_writestring("This is a minimal kernel implementation.\n");
    terminal_writestring("\n");
    
    // Also send output to serial for debugging
    serial_write("KERNEL: Hello from our custom unikernel!\n");
    serial_write("KERNEL: This is a minimal kernel implementation.\n");
    serial_write("KERNEL: \n");
    
    // Initialize network
    serial_write("KERNEL: Initializing network...\n");
    terminal_writestring("Initializing network...\n");
    if (network_init() == 0) {
        serial_write("KERNEL: Network initialized successfully\n");
        terminal_writestring("Network initialized successfully\n");
    } else {
        serial_write("KERNEL: Failed to initialize network\n");
        terminal_writestring("Failed to initialize network\n");
    }
    
    // Test our API handlers
    serial_write("KERNEL: Testing API handlers...\n");
    test_api_handlers();
    
    // Test LLM functionality
    serial_write("KERNEL: Testing LLM functionality...\n");
    test_llm();
    serial_write("KERNEL: Finished testing LLM functionality\n");
    
    // Test CUDA functionality
    serial_write("KERNEL: Testing CUDA functionality...\n");
    test_cuda();
    
    // Test error handling functionality
    serial_write("KERNEL: Testing error handling functionality...\n");
    test_error_handling();
    
    terminal_writestring("\nAll tests completed. Entering network loop...\n");
    serial_write("KERNEL: All tests completed. Entering network loop...\n");
    
    // Enter networking loop (never returns)
    network_loop();
}
