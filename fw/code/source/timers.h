#ifndef  _TIMERS_H_
#define  _TIMERS_H_

#include <stdint.h>
#include <globals.h>
#include <buddy.h>

// Timer 0
#define TIMER0_LOW_PERIOD 21
#define TIMER0_HIGH_PERIOD 65535000

#define DEFAULT_PERIOD 1e-3
#define USEC_TO_TICKS(USECS,CLOCK) (((USECS*1e-6)) * SYSCLK)

// Timer2
#define TIMER_PRESCALER            12   // Based on Timer2 CKCON and TMR2CN
                                        // settings

#define LED_TOGGLE_RATE            1   // LED toggle rate in seconds

#define AUX_FS 0xFFFF					// 0xFFFF * 1/(SYSCLK/12)
										// 0xFFFF * 1/(48e6/12)
										// 0xFFFF * 250e-9 = 16.38e-3

/*
#define TIMER2_RELOAD 0					// 0xFFFF = 65535
#define TIMER2_LOOPS LED_TOGGLE_RATE / (AUX_FS * (1.0f / (SYSCLK/TIMER_PRESCALER)))
*/

void Timer0_Init (void);
//void Timer2_Init (void);
void Timer2_Set_Period (uint16_t period);
void Timer0_Set_Period (uint32_t period);

#endif