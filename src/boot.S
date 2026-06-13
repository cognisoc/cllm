/*
 * boot.S - Simplified bootloader for QEMU compatibility
 */

# Mark the object as not requiring an executable stack.
.section .note.GNU-stack, "", @progbits

# Multiboot header - simplified for QEMU
.section .multiboot
.align 4

# Multiboot magic number and flags
.long 0x1BADB002                    # Magic number
.long 0x00000003                   # Flags (page align + memory info)
.long -(0x1BADB002 + 0x00000003)   # Checksum

# Stack section - larger stack for debugging
.section .bss
.align 16
stack_bottom:
.skip 65536  # 64KB stack
stack_top:

# Text section
.section .text
.global _start
.global kernel_main

# Entry point - QEMU compatible
_start:
    # Set up stack pointer
    mov $stack_top, %esp
    
    # Clear flags
    cld
    
    # Zero out BSS section
    mov $__bss_start, %edi
    mov $__bss_end, %ecx
    sub %edi, %ecx
    shr $2, %ecx
    xor %eax, %eax
    rep stosl
    
    # Initialize serial port for early debugging
    mov $0x3F8, %edx
    mov $0x00, %al
    outb %al, %dx
    mov $0x80, %al
    outb %al, %dx
    mov $0x03, %al
    outb %al, %dx
    mov $0x00, %al
    outb %al, %dx
    mov $0x03, %al
    outb %al, %dx
    mov $0xC7, %al
    outb %al, %dx
    
    # Send boot message to serial
    mov $0x3F8, %edx
    mov $'B', %al
    outb %al, %dx
    mov $'O', %al
    outb %al, %dx
    mov $'O', %al
    outb %al, %dx
    mov $'T', %al
    outb %al, %dx
    mov $'\r', %al
    outb %al, %dx
    mov $'\n', %al
    outb %al, %dx
    
    # Call kernel main function
    call kernel_main
    
    # Halt if kernel_main returns
halt:
    cli
    hlt
    jmp halt