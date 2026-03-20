/*
 * boot32.S - 32-bit assembly bootloader for our unikernel
 * Implements multiboot specification
 */

# Multiboot header
.section .multiboot
.align 4

# Multiboot flags
.set MULTIBOOT_PAGE_ALIGN, 1<<0
.set MULTIBOOT_MEMORY_INFO, 1<<1
.set MULTIBOOT_HEADER_FLAGS, MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO

# Multiboot magic number
.long 0x1BADB002                    # Magic number required by multiboot spec
.long MULTIBOOT_HEADER_FLAGS        # Flags
.long -(0x1BADB002 + MULTIBOOT_HEADER_FLAGS) # Checksum (magic + flags + checksum = 0)

# AOUT kludge - not used, but required for some bootloaders
.long 0, 0, 0, 0, 0

# Stack section
.section .bss
.align 16
stack_bottom:
.skip 16384  # 16KB stack
stack_top:

# Text section
.section .text
.global _start
.global kernel_main

# Entry point
_start:
    # Set up stack with proper alignment
    mov $stack_top, %esp
    and $0xFFFFFFF0, %esp  # Align to 16-byte boundary
    
    # Call kernel main function
    call kernel_main
    
    # Halt if kernel_main returns
halt:
    cli
    hlt
    jmp halt