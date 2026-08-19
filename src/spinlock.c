/*
 * spinlock.c - Test-and-set spinlock for single-CPU x86
 */

#include "spinlock.h"
#include "interrupts.h"

void spinlock_init(spinlock_t* lock) {
    if (!lock) return;
    lock->locked = 0;
    lock->interrupts_were_enabled = false;
}

bool spinlock_trylock(spinlock_t* lock) {
    if (!lock) return false;

    unsigned int got = 1;

    // xchg with memory operand is atomic on x86.
    __asm__ volatile(
        "xchg %0, %1"
        : "=r"(got), "+m"(lock->locked)
        : "0"(got)
        : "memory"
    );

    if (got == 0) {
        lock->interrupts_were_enabled = interrupts_disable_save();
        return true;
    }
    return false;
}

void spinlock_lock(spinlock_t* lock) {
    if (!lock) return;
    while (!spinlock_trylock(lock)) {
        // On single-core QEMU we could just pause, but actively spin with
        // interrupts enabled so we don't deadlock if an ISR needs the lock.
        interrupts_enable();
        __asm__ volatile("pause");
        interrupts_disable();
    }
}

void spinlock_unlock(spinlock_t* lock) {
    if (!lock) return;

    bool restore = lock->interrupts_were_enabled;
    lock->locked = 0;

    // Full memory barrier so prior stores are visible before the lock is
    // observable as released.
    __asm__ volatile("" ::: "memory");

    interrupts_restore(restore);
}

bool spinlock_is_locked(const spinlock_t* lock) {
    if (!lock) return false;
    return lock->locked != 0;
}
