/* Linker script for QEMU-compatible unikernel */
OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
    /* Multiboot requires kernel to be loaded at 1MB */
    . = 0x00100000;

    /* Text section - multiboot header must be within first 8KB */
    .text ALIGN(4) : {
        *(.multiboot)
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    /* Data section */
    .data : {
        *(.data)
    }

    /* BSS section - zero-initialized data */
    .bss : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.GNU-stack)
    }
}