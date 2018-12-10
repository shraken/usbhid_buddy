#ifndef  _TIMERS_H_
#define  _TIMERS_H_

#include <compiler_defs.h>
#include <c8051f380.h>
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

extern uint8_t __data adc_channel_index;
extern uint8_t adc_channel_count;
extern uint8_t adc_mux_tbl_n[MAX_ANALOG_INPUTS];
extern uint8_t adc_mux_tbl_p[MAX_ANALOG_INPUTS];

extern bit SMB_BUSY;

void timers_init(void);

void timer0_init(void);
void timer3_init(void);

void timer2_init (void);
void timer2_set_period(uint32_t period);

void timer2_isr (void) __interrupt (INTERRUPT_TIMER2);
void timer3_isr (void) __interrupt (INTERRUPT_TIMER3);

#endif