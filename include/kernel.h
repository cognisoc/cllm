/*
 * kernel.h - Kernel header file
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>

// VGA text buffer constants
#define VGA_TEXT_WIDTH 80
#define VGA_TEXT_HEIGHT 25
#define VGA_BUFFER (unsigned short*)0xB8000

// Function declarations
void terminal_init(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void serial_write(const char* data);
void serial_init(void);

// Memory allocation
void* malloc(size_t size);
void free(void* ptr);

#endif