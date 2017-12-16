/**
 * @file timers.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-09-26
 * @brief Timer configuration routines.  The timer is used
 *				 as main time keeper for stream operations.  With
 *				 DAC/PWM modes a sample from the frame is output and
 *				 with ADC mode a sample is pulled and copied to the
 *				 buffer.  
 *			
 */
 
#ifndef  _TIMERS_H_
#define  _TIMERS_H_

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