/*
 * boot.S - Assembly bootloader for our unikernel
 * Implements multiboot specification
 */

# Multiboot header
.section .multiboot
.align 4

# Multiboot magic number
.long 0x1BADB002          # Magic number required by multiboot spec
.long 0x0                 # Flags (0 = no special flags)
.long -(0x1BADB002 + 0x0) # Checksum (magic + flags + checksum = 0)

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
    # Set up stack
    mov $stack_top, %esp
    
    # Call kernel main function
    call kernel_main
    
    # Halt if kernel_main returns
halt:
    cli
    hlt
    jmp halt