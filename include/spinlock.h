#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdbool.h>

// Simple test-and-set spinlock for single-CPU x86.
//
// Because the unikernel runs on a single core, the lock primarily protects
// against reentrancy from interrupt handlers.  Therefore lock acquisition
// disables interrupts and stores the previous interrupt state; unlock restores
// it.  This avoids deadlocks where an ISR attempts to acquire a lock held by
// the interrupted context.

typedef struct {
    volatile unsigned int locked;
    bool interrupts_were_enabled;
} spinlock_t;

#define SPINLOCK_INIT {0, false}

// Initialize a spinlock.
void spinlock_init(spinlock_t* lock);

// Acquire the lock, disabling interrupts until release.
void spinlock_lock(spinlock_t* lock);

// Release the lock and restore previous interrupt state.
void spinlock_unlock(spinlock_t* lock);

// Try to acquire the lock without blocking.  Returns true on success.
bool spinlock_trylock(spinlock_t* lock);

// Return true if the lock is currently held.
bool spinlock_is_locked(const spinlock_t* lock);

#endif // SPINLOCK_H
