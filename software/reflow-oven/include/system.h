#ifndef SYSTEM_H
#define SYSTEM_H


#include <stdint.h>


void system_clock_init_pll_hse_72(void);
void system_time_init(void);
uint64_t system_time_get_us(void);
void system_time_wait_us(uint64_t us);


#endif // SYSTEM_H