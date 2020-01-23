#ifndef FIXED_PRIORITY_H
#define FIXED_PRIORITY_H

#include <asm/percpu.h>

// called from arm-common/control.c
void on_arch_park_cpu(unsigned int cpu_id);

// called from OS via hvc
long fixed_priority_hypercall(struct per_cpu *cpu_data, unsigned long action_code);

#endif /* FIXED_PRIORITY_H */