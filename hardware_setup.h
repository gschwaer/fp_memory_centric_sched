#ifndef __HARDWARE_SETUP_H__
#define __HARDWARE_SETUP_H__

void disable_prefetcher(void);
void disable_predictor(void);
void enable_cycle_counter(void);
uint64_t get_cycle_counter_value(void);

#endif /* __HARDWARE_SETUP_H__ */
