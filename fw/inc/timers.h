/** @file timers.h
 *  @brief Function prototypes for the timer routines.
 *
 */
 
#ifndef _C8051F380_TIMERS_H
#define _C8051F380_TIMERS_H

#include <stdint.h>
#include <compiler_defs.h>
#include <c8051f380.h>
#include <globals.h>

// shraken: tuned timer0 hi/lo values such that
// timer overflows at a 1msec interval given a
// SYSCLK/4.  
#define TIMER0_RELOAD_HIGH  244        // Timer0 High register
#define TIMER0_RELOAD_LOW 0            // Timer0 Low register
 
void Timer0_ISR (void) __interrupt (INTERRUPT_TIMER0);
void Timer0_Init(void);

#endif