#pragma once

#define x_lock_cpu()
#define x_unlock_cpu()
#define cmpxchg4(p,o,n) arch_atomic_val_compare_and_exchange((uint32_t*)(p),o,n)
#define CYCLES_PER_RELAX_LOOP 7
static inline void
relax(int iterations)
{
    for (/*above*/; iterations > 0; iterations--)
            __insn_mfspr(0x2709/*SPR_PASS*/);
	        __asm__ __volatile__("" ::: "memory");
		}
#include <arch/atomic.h>


/**
 * Ticket spin lock with backoff (from linux 4.5)
 */

#define u32 uint32_t
#  define unlikely(x)	(__builtin_expect(!!(x), 0))
#define wmb() __insn_mf()

// From 'arch/tile/include/asm/spinlock_types.h'

/* Low 15 bits are "next"; high 15 bits are "current". */
typedef struct arch_spinlock {
	unsigned int lock;
} arch_spinlock_t;

#define __ARCH_SPIN_LOCK_UNLOCKED	{ 0 }

// From 'arch/tile/include/asm/spinlock_64.h'

/* Shifts and masks for the various fields in "lock". */
#define __ARCH_SPIN_CURRENT_SHIFT	17
#define __ARCH_SPIN_NEXT_MASK		0x7fff
#define __ARCH_SPIN_NEXT_OVERFLOW	0x8000

/*
 * Return the "current" portion of a ticket lock value,
 * i.e. the number that currently owns the lock.
 */
static inline u32 arch_spin_current(u32 val)
{
	return val >> __ARCH_SPIN_CURRENT_SHIFT;
}

/*
 * Return the "next" portion of a ticket lock value,
 * i.e. the number that the next task to try to acquire the lock will get.
 */
static inline u32 arch_spin_next(u32 val)
{
	return val & __ARCH_SPIN_NEXT_MASK;
}

/* Bump the current ticket so the next task owns the lock. */
static inline void arch_spin_unlock(arch_spinlock_t *lock)
{
	wmb();  /* guarantee anything modified under the lock is visible */
	__insn_fetchadd4(&lock->lock, 1U << __ARCH_SPIN_CURRENT_SHIFT);
}

// From 'arch/tile/lib/spinlock_64.c'

/*
 * Read the spinlock value without allocating in our cache and without
 * causing an invalidation to another cpu with a copy of the cacheline.
 * This is important when we are spinning waiting for the lock.
 */
static inline u32 arch_spin_read_noalloc(void *lock)
{
	return cmpxchg4(lock, -1, -1);
}

/*
 * Wait until the high bits (current) match my ticket.
 * If we notice the overflow bit set on entry, we clear it.
 */
static inline void arch_spin_lock_slow(arch_spinlock_t *lock, u32 my_ticket, bool_t preemption)
{
	if (unlikely(my_ticket & __ARCH_SPIN_NEXT_OVERFLOW)) {
		__insn_fetchand4(&lock->lock, ~__ARCH_SPIN_NEXT_OVERFLOW);
		my_ticket &= ~__ARCH_SPIN_NEXT_OVERFLOW;
	}

	for (;;) {
		u32 val = arch_spin_read_noalloc(lock);
		u32 delta = my_ticket - arch_spin_current(val);
		if (delta == 0)
			return;
		if (preemption) x_unlock_cpu();
		relax((128 / CYCLES_PER_RELAX_LOOP) * delta);
		if (preemption) x_lock_cpu();
	}
}

/*
 * Check the lock to see if it is plausible, and try to get it with cmpxchg().
 */
static inline int arch_spin_trylock(arch_spinlock_t *lock)
{
	u32 val = arch_spin_read_noalloc(lock);
	if (unlikely(arch_spin_current(val) != arch_spin_next(val)))
		return 0;
	return cmpxchg4(&lock->lock, val, (val + 1) & ~__ARCH_SPIN_NEXT_OVERFLOW)
		== val;
}

// From 'arch/tile/include/asm/spinlock_64.h' again

/* Grab the "next" ticket number and bump it atomically.
 * If the current ticket is not ours, go to the slow path.
 * We also take the slow path if the "next" value overflows.
 */
static inline void arch_spin_lock(arch_spinlock_t *lock, bool_t preemption)
{
	u32 val = __insn_fetchadd4(&lock->lock, 1);
	u32 ticket = val & (__ARCH_SPIN_NEXT_MASK | __ARCH_SPIN_NEXT_OVERFLOW);
	if (unlikely(arch_spin_current(val) != ticket))
		arch_spin_lock_slow(lock, ticket, preemption);
}


#define PREFIX_SPIN_LOCK_NAME(x) backoff##_##x
#define LOCK PREFIX_SPIN_LOCK_NAME(lock)

typedef arch_spinlock_t LOCK /*__attribute__((aligned(CHIP_L2_LINE_SIZE())))*/;

static inline
void PREFIX_SPIN_LOCK_NAME(initialize_lock)(LOCK *p_lock) {
	__insn_mf();
	p_lock->lock = 0;
}

static inline
void PREFIX_SPIN_LOCK_NAME(acquire_lock)(LOCK *p_lock) {
    arch_spin_lock(p_lock, true);
}

/* IMPORTANT: 'true' means 'failure' as in x_try_lock_spin() */
static inline
bool_t PREFIX_SPIN_LOCK_NAME(try_acquire_lock)(LOCK *p_lock) {
	return(!arch_spin_trylock(p_lock));
}

static inline
void PREFIX_SPIN_LOCK_NAME(acquire_lock_without_preemption)(LOCK *p_lock) {
    arch_spin_lock(p_lock, false);
}

// Reference: 'arch/arm_gcc/mpcore/chip_config_tool.h'
#define backoff_ACQUIRE_NESTED_LOCK do { \
		int iterations = 0; \
		PCB     *p_pcb; \
		while(backoff_try_acquire_lock(p_lock)) { \
			p_pcb = get_my_p_pcb(); \
			if (p_pcb->p_firstlock == NULL) { \
				return true; \
			} \
			delay_backoff(iterations++); \
		} \
		return false; \
} while(0)

static inline
void PREFIX_SPIN_LOCK_NAME(release_lock)(LOCK *p_lock) {
    arch_spin_unlock(p_lock);
}

#undef LOCK
#undef PREFIX_SPIN_LOCK_NAME
