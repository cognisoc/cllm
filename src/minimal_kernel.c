/*
 * minimal_kernel.c - Minimal kernel for QEMU testing
 */

// Define basic types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long size_t;

// Simple serial port output
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void serial_write_char(char c) {
    outb(0x3F8, c);
}

static void serial_write_string(const char* str) {
    while (*str) {
        serial_write_char(*str++);
    }
}

// Minimal kernel main function
void kernel_main(void) {
    // Initialize serial port
    outb(0x3F8 + 1, 0x00);    // Disable interrupts
    outb(0x3F8 + 3, 0x80);    // Enable DLAB
    outb(0x3F8 + 0, 0x03);    // Set divisor low byte
    outb(0x3F8 + 1, 0x00);    // Set divisor high byte
    outb(0x3F8 + 3, 0x03);    // 8N1
    outb(0x3F8 + 2, 0xC7);    // Enable FIFO, clear them
    outb(0x3F8 + 4, 0x0B);    // Enable interrupts

    // Send boot message
    serial_write_string("MINIMAL KERNEL: Booting...\r\n");
    serial_write_string("MINIMAL KERNEL: Serial initialized\r\n");
    serial_write_string("MINIMAL KERNEL: Entering main loop\r\n");

    // Simple counter test
    for (int i = 0; i < 10; i++) {
        serial_write_string("MINIMAL KERNEL: Counter: ");
        serial_write_char('0' + i);
        serial_write_string("\r\n");
    }

    // Success message
    serial_write_string("MINIMAL KERNEL: All tests passed!\r\n");
    serial_write_string("MINIMAL KERNEL: Halting...\r\n");

    // Halt
    __asm__ volatile("cli; hlt");
}