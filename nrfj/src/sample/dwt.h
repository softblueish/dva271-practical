
#ifndef DWT_H_
#define DWT_H_

#include <nrf.h>

// init DWT (Data Watchpoint and Trace unit) for cycle counting
void dwt_init();

// get current cycle count
uint32_t dwt_get_cycle();

// delay until the cycle counter reaches the specified value
void dwt_delay_until_cycle(uint32_t cycle);

#endif // DWT_H_