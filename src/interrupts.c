/*
 * interrupts.c - x86 interrupt flag control
 */

#include "interrupts.h"

void interrupts_disable(void) {
    __asm__ volatile("cli");
}

void interrupts_enable(void) {
    __asm__ volatile("sti");
}

bool interrupts_enabled(void) {
    uint32_t eflags;
    __asm__ volatile(
        "pushf\n"
        "pop %0"
        : "=r"(eflags)
    );
    return (eflags & 0x200) != 0;
}

bool interrupts_disable_save(void) {
    bool was_enabled = interrupts_enabled();
    interrupts_disable();
    return was_enabled;
}

void interrupts_restore(bool was_enabled) {
    if (was_enabled) {
        interrupts_enable();
    }
}
