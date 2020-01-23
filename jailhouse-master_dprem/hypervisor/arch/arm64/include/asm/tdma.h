#ifndef TDMA_H
#define TDMA_H

#include <jailhouse/types.h>

#include <asm/percpu.h>

// called from hardware setup gic-v2.c
void tdma_on_cell_create( struct cell *cell );
void tdma_on_cell_destroy( struct cell *cell );
void tdma_setup_irq_target( u8 local_irq_target );

// called from irqchip.c to suspend the core if necessary
void tdma_block_if_needed(struct per_cpu *cpu_data);

// called from arm-common/control.c to check if a given interrupt was associated
// with the TDMA
bool tdma_handle_interrupt(struct per_cpu *cpu_data, u32 irqn);

// called from OS via hvc
long tdma_hypercall(struct per_cpu *cpu_data, unsigned long action_code);

#endif /* TDMA_H */