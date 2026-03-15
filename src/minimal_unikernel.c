/*
 * Minimal bare-metal program for x86_64
 * This program will run directly on the hardware (or QEMU) without an OS
 */

// Entry point for the unikernel
void _start(void) {
    // For now, we'll just try to print a message
    // In a real unikernel, we would need to implement our own console output
    
    // Halt the CPU
    while (1) {
        __asm__("hlt");
    }
}