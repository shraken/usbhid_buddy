/** @file millis.h
 *  @brief Function prototypes for the Millis timekeeper.
 *
 */
 
#ifndef _C8051F380_MILLIS_H
#define _C8051F380_MILLIS_H

#include <compiler_defs.h>
#include <c8051f380.h>

uint32_t millis(void);
void millisDelay(uint32_t DelayTime);
uint32_t get_millis_tick_count(void);

#endif