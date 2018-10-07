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
#include <adc.h>
#include <buddy.h>

// Timer 2
#define TIMER2_LOW_PERIOD 21		    // in nsec, 21 nsec
#define TIMER2_HIGH_PERIOD 16383750		// in nsec, 16.38375 msec

#define DEFAULT_TIMER2_HIGH_PERIOD 0xF0
#define DEFAULT_TIMER2_LOW_PERIOD 0x5F

extern buddy_ctx_t buddy_ctx;

extern uint8_t data adc_channel_index;
extern uint8_t adc_channel_count;
extern uint8_t adc_mux_tbl_n[MAX_ANALOG_INPUTS];
extern uint8_t adc_mux_tbl_p[MAX_ANALOG_INPUTS];

extern bit SMB_BUSY;

void timers_init(void);

void timer0_init(void);
void timer3_init(void);

void timer2_init (void);
void timer2_set_period(uint32_t period);

#endif