/*
 * Sleep functionalities
 *
 * Copyright (C) 2018 Università di Modena e Reggio Emilia
 *
 * Authors:
 *  Luca Miccio <lucmiccio@gmail.com>
 *
 * N.B: this file should be used for testing only
 */
#ifndef HERCULES_TIMER_H
#define HERCULES_TIMER_H

#define ARM64_READ_SYSREG(reg) \
({ \
    u64 _val; \
    __asm__ volatile("mrs %0," #reg : "=r" (_val)); \
    _val; \
})

/* Sleep interface */

// This value depends on the Hardware used
// 32 is for TX2
// The formula to calculate this value is
// (1ms)1000000 / Timer_freq (32 MhZ on TX2)
#define COUNTS_PER_MSECOND 32
static void sleep_common(u32 n, u32 count)
{
        u64 tCur,tEnd;
        tCur = ARM64_READ_SYSREG(CNTPCT_EL0);
        tEnd = tCur + (((u64)n) * count);
        do {
                tCur = ARM64_READ_SYSREG(CNTPCT_EL0);
        } while (tCur < tEnd);
}
int usleep(unsigned long useconds);
int sleep(unsigned long seconds);
int usleep(unsigned long useconds)
{
        sleep_common((u32)useconds, COUNTS_PER_MSECOND);
        return 0;
}
int sleep(unsigned long seconds)
{
        usleep(seconds*1000000);
        return 0;
}

#endif /* HERCULES_TIMER_H */
