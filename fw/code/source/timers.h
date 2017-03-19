#ifndef  _TIMERS_H_
#define  _TIMERS_H_

#include <stdint.h>
#include <globals.h>

#define TIMER0_LOW_PERIOD 21
#define TIMER0_HIGH_PERIOD 65535000

#define DEFAULT_PERIOD 1e-3
#define USEC_TO_TICKS(USECS,CLOCK) (((USECS*1e-6)) * SYSCLK)

void Timer2_Init (void);
void Timer2_Set_Period (uint16_t period);
void Timer0_Set_Period (uint32_t period);

#endif