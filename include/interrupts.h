#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdbool.h>
#include <stdint.h>

// x86 interrupt control primitives.
// These are intentionally thin wrappers around the CLI/STI/POPF instructions
// so the rest of the kernel can reason about critical sections without
// embedding inline assembly everywhere.

// Disable maskable interrupts.
void interrupts_disable(void);

// Enable maskable interrupts.
void interrupts_enable(void);

// Return true if interrupts are currently enabled.
bool interrupts_enabled(void);

// Save the current interrupt flag and disable interrupts.
// Returns the previous state (true == interrupts were enabled).
bool interrupts_disable_save(void);

// Restore interrupt flag to the state returned by interrupts_disable_save().
void interrupts_restore(bool was_enabled);

#endif // INTERRUPTS_H
