#ifndef  _TIMERS_H_
#define  _TIMERS_H_

#include <compiler_defs.h>
#include <c8051f380.h>
#include <stdint.h>
#include <stdio.h>
#include <c8051f3xx.h>
#include "globals.h"
#include "process.h"
#include "adc.h"
#include "io.h"
#include "i2c.h"
#include "gpio.h"
#include "utility.h"
#include "process.h"
#include "buddy_common.h"

// Timer 2
#define TIMER2_LOW_PERIOD 21		    // in nsec, 21 nsec
#define TIMER2_HIGH_PERIOD 16383750		// in nsec, 16.38375 msec

#define DEFAULT_TIMER2_HIGH_PERIOD 0xF0
#define DEFAULT_TIMER2_LOW_PERIOD 0x5F

extern uint8_t timer2_flag;

void timers_init(void);

void timer0_init(void);
void timer3_init(void);

void timer2_init (void);
void timer2_set_period(uint32_t period);

void timer2_isr (void) __interrupt (INTERRUPT_TIMER2);
void timer3_isr (void) __interrupt (INTERRUPT_TIMER3);

#endif