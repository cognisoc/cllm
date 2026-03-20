/* Basic linker script for unikernel */
ENTRY(_start)

SECTIONS
{
    . = 1M;  /* Load kernel at 1MB */

    .text : {
        *(.multiboot)
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}