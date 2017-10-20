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
#include <globals.h>
#include <buddy.h>

// Timer 0
#define TIMER0_LOW_PERIOD 21					// 21 nsec
#define TIMER0_HIGH_PERIOD 65535000		// 65.53 sec

#define DEFAULT_TIMER0_HIGH_PERIOD 0xF0
#define DEFAULT_TIMER0_LOW_PERIOD 0x5F

void Timer0_Init (void);
void Timer0_Set_Period (uint32_t period);

#endif