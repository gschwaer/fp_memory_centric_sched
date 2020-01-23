/*
 * Jailhouse AArch64 support
 *
 * Copyright (C) 2015 Huawei Technologies Duesseldorf GmbH
 *
 * Authors:
 *  Antonios Motakis <antonios.motakis@huawei.com>
 *
 * Spinlock implementation copied from
 * arch/arm64/include/asm/spinlock.h in Linux
 *
 * Copyright (C) 2012 ARM Ltd.
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 */

#ifndef _JAILHOUSE_ASM_SPINLOCK_H
#define _JAILHOUSE_ASM_SPINLOCK_H

#include <jailhouse/types.h>

#define DEFINE_SPINLOCK(name)	spinlock_t (name)
#define TICKET_SHIFT	16

/* TODO: fix this if we add support for BE */
typedef struct {
	u16 owner;
	u16 next;
} spinlock_t __attribute__((aligned(4)));

static inline void spin_lock(spinlock_t *lock)
{
	unsigned int tmp;
	spinlock_t lockval, newval;

	asm volatile(
	/* Atomically increment the next ticket. */
"	prfm	pstl1strm, %3\n"
"1:	ldaxr	%w0, %3\n"
"	add	%w1, %w0, %w5\n"
"	stxr	%w2, %w1, %3\n"
"	cbnz	%w2, 1b\n"
	/* Did we get the lock? */
"	eor	%w1, %w0, %w0, ror #16\n"
"	cbz	%w1, 3f\n"
	/*
	 * No: spin on the owner. Send a local event to avoid missing an
	 * unlock before the exclusive load.
	 */
"	sevl\n"
"2:	wfe\n"
"	ldaxrh	%w2, %4\n"
"	eor	%w1, %w2, %w0, lsr #16\n"
"	cbnz	%w1, 2b\n"
	/* We got the lock. Critical section starts here. */
"3:"
	: "=&r" (lockval), "=&r" (newval), "=&r" (tmp), "+Q" (*lock)
	: "Q" (lock->owner), "I" (1 << TICKET_SHIFT)
	: "memory");
}

static inline void spin_unlock(spinlock_t *lock)
{
	asm volatile(
"	stlrh	%w1, %0\n"
	: "=Q" (lock->owner)
	: "r" (lock->owner + 1)
	: "memory");
}


static inline void priospin_lock(spinlock_t *lock, unsigned int prio)
{
        unsigned int tmp;
        spinlock_t lockval;

        asm volatile(
"       prfm    pstl1strm, %2\n"
"1:     ldaxr   %w0, %2\n"                 // exclusive load (register)
"       cbnz    %w0, 2f\n"                 // if lock is in use, update the queue
"       stxr    %w1, %w4, %2\n"            // store exclusive lock owner (register)
"       cbz     %w1, 4f\n"                 // store succeeded, lock obtained, go to the critical section
"       b       1b\n"                      // store failed, retry
"2:     orr     %w0, %w0, %w4, lsl #16\n"  // update queue
"       stxr    %w1, %w0, %2\n"            // exlusive store
"       cbnz    %w1, 1b\n"                 // store failed, retry
"       sevl\n"                            // send a local event to avoid missing an unlock before the exclusive load
"3:     wfe\n"                             // spin
"       ldaxrh  %w0, %3\n"                 // get the owner
"       eor     %w0, %w4, %w0\n"           // check if it's my turn
"       cbnz    %w0, 3b\n"
        /* We got the lock. Critical section starts here. */
"4:"
        : "=&r" (lockval), "=&r" (tmp), "+Q" (*lock)
        : "Q" (lock->owner), "r" (prio)
        : "memory");
}


static inline void priospin_unlock(spinlock_t *lock, unsigned int prio)
{

        unsigned int tmp1, tmp2;
        spinlock_t lockval;

        asm volatile(
"       lsl     %w4, %w4, #16\n"
"1:     ldaxr   %w0, %3\n"
"       and     %w0, %w0, #0xFFFF0000\n"   // reset owner
"       bic     %w0, %w0, %w4\n"           // remove from queue the current id
"       cbz     %w0, 2f\n"                 // queue is empty
"       clz     %w1, %w0\n"                // count leading zeros LZ
"       mov     %w2, #0xF\n"
"       sub     %w1, %w2, %w1\n"           // 16-LZ
"       mov     %w2, #1\n"
"       lsl     %w1, %w2, %w1\n"           // 1<<(16-LZ)
"       orr     %w0, %w0, %w1\n"           // W0 = W0 or (1<<(16-LZ))
"2:     stxr    %w2, %w0, %3\n"            // clear the owner and the queue
"       cbnz    %w2, 1b\n"                 // store failed, retry
        : "=&r" (lockval), "=&r" (tmp1), "=&r" (tmp2), "+Q" (*lock)
        : "r" (prio)
        : "memory");
}


#endif /* !_JAILHOUSE_ASM_SPINLOCK_H */
