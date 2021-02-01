#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

void system_clock_init_pll_hse_72(void);
uint32_t system_clock_get_hclk(void);
uint32_t system_clock_get_pclk1(void);
uint32_t system_clock_get_pclk2(void);
void system_time_init(void);
uint64_t system_usec2tick(uint64_t usec);
uint64_t system_time_get_tick(void);
uint64_t system_time_get_usec(void);
void system_time_wait_usec(uint64_t usec);

#endif // SYSTEM_H