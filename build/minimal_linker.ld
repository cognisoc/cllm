/* Minimal linker script for QEMU testing */
ENTRY(_start)

SECTIONS
{
    /* Multiboot requires kernel to be loaded at 1MB */
    . = 0x00100000;

    /* Multiboot header must be in first 8KB */
    .multiboot : {
        *(.multiboot)
    }

    /* Text section */
    .text : {
        *(.text)
    }

    /* Read-only data */
    .rodata : {
        *(.rodata)
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