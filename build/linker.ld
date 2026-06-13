/* Linker script for QEMU-compatible unikernel */
OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5); /* RX */
    rodata PT_LOAD FLAGS(4); /* R */
    data PT_LOAD FLAGS(6); /* RW */
}

SECTIONS
{
    /* Multiboot requires kernel to be loaded at 1MB */
    . = 0x00100000;

    /* Text section - multiboot header must be within first 8KB */
    .text ALIGN(4) : {
        *(.multiboot)
        *(.text)
        *(.text.*)
    } :text

    /* Read-only data */
    .rodata ALIGN(4) : {
        *(.rodata)
        *(.rodata.*)
    } :rodata

    /* Data section */
    .data ALIGN(4) : {
        *(.data)
    } :data

    /* BSS section - zero-initialized data */
    .bss ALIGN(4) : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    } :data

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.GNU-stack)
    }
}