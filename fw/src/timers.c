#include <stdio.h>
#include <stdint.h>
#include <c8051f3xx.h>
#include <timers.h>
#include <buddy.h>
#include <globals.h>
#include <process.h>
#include <adc.h>
#include <io.h>
#include <gpio.h>
#include <utility.h>

uint8_t timer0_flag = 0;

static uint8_t timer0_low_set;
static uint8_t timer0_high_set;

volatile uint32_t _millisCounter = 0;

void timer_init(void)
{
	// set to 0xF05F (4000) ticks at 0.25 usec timer period
	// for equivalent interrupt of 1000 usec.
	
	TH0 = DEFAULT_TIMER0_HIGH_PERIOD;
	TL0 = DEFAULT_TIMER0_LOW_PERIOD;
	
    timer0_low_set = DEFAULT_TIMER0_LOW_PERIOD;
    timer0_high_set = DEFAULT_TIMER0_HIGH_PERIOD;

	// Mode1: 16-bit counter/timer
	TMOD |= 0x01;
	
	// Timer0 uses SYSCLK / 12
	CKCON &= ~(0x07);
	CKCON |= 0x04;
	
	// Timer0 interrupt enabled
	ET0 = 1;
	
	// Timer0 ON
	TCON |= 0x10;
}

void timer_set_period(uint32_t period)
{
	uint16_t timer_set;

	// Disable Timer0
	TCON &= ~(0x10);
	
	// Timer0 interrupt disabled
	ET0 = 0;
	
	debug(("timer_set_period, nsec period = %lu\r\n", period));
	
	// bound period by lower value of 20.83 nsec (21 nsec)
	// and upper value of 65535000 nsec.
	
	if (period < TIMER0_LOW_PERIOD) {
		period = TIMER0_LOW_PERIOD;
	} else if (period > TIMER0_HIGH_PERIOD) {
		period = TIMER0_HIGH_PERIOD;
	}
	
	CKCON &= ~(0x07);
		
	// set Timer0 clock base and 16-bit timer comparator value
	
	if ((period < (((1.0 /SYSCLK) * 0xFFFF) / 1e-9)) &&
	    (period > (((1.0 /SYSCLK) * 0x0001) / 1e-9))) {
		timer_set = 0xFFFF - (period * 1e-9) * SYSCLK;
		timer0_high_set = ((timer_set & 0xFF00) >> 8);
		timer0_low_set = (timer_set & 0x00FF);
		debug(("Timer0 set region 1 with %u\r\n", timer_set));
				 
		// Timer0 uses SYSCLK
		//CKCON |= 0x04;
		//CKCON &= 0xFB; // clear T0 (timer 0 clock select)
		//CKCON |= 0x04; // set T0 (timer 0 uses system clock)
		CKCON |= 0x04;
	} else if ((period < (((1.0 /(SYSCLK/4)) * 0xFFFF) / 1e-9)) &&
	    (period > (((1.0 /(SYSCLK/4)) * 0x0001) / 1e-9))) {
		timer_set = 0xFFFF - (period * 1e-9) * (SYSCLK/4);
		timer0_high_set = ((timer_set & 0xFF00) >> 8);
		timer0_low_set = (timer_set & 0x00FF);
		debug(("Timer0 set region 2 with %u\r\n", timer_set));
				 
		// Timer0 uses SYSCLK / 4
		//CKCON |= 0x01;
		CKCON &= 0xF8; // clear T0 and SCA[1:0]
		CKCON |= 0x01; 
	} else if ((period < (((1.0 /(SYSCLK/12)) * 0xFFFF) / 1e-9)) &&
	    (period > (((1.0 /(SYSCLK/12)) * 0x0001) / 1e-9))) {
		timer_set = 0xFFFF - (period * 1e-9) * (SYSCLK/12);
		timer0_high_set = ((timer_set & 0xFF00) >> 8);
		timer0_low_set = (timer_set & 0x00FF);
		debug(("Timer0 set region 3 with %u\r\n", timer_set));
				 
		// Timer0 uses SYSCLK / 12
		//CKCON |= 0x00;
		CKCON &= 0xF8; // clear T0 and SCA[1:0]
		CKCON |= 0x00; 
	} else if ((period < (((1.0 /(SYSCLK/48)) * 0xFFFF) / 1e-9)) &&
	    (period > (((1.0 /(SYSCLK/48)) * 0x0001) / 1e-9))) {
		timer_set = 0xFFFF - (period * 1e-9) * (SYSCLK/48);
		timer0_high_set = ((timer_set & 0xFF00) >> 8);
		timer0_low_set = (timer_set & 0x00FF);
	   	debug(("Timer0 set region 4 with %u\r\n", timer_set));
				 
		// Timer0 uses SYSCLK / 48
		//CKCON |= 0x02;
		CKCON &= 0xF8; // clear T0 and SCA[1:0]
		CKCON |= 0x02; 
	} else {
		debug(("Timer0 region not settable\r\n"));
			
		// Timer0 uses SYSCLK / 48
		CKCON |= 0x02;
			
		timer0_high_set = 0x00;
		timer0_low_set = 0x00;
	}
		
	TH0 = timer0_high_set;
	TL0 = timer0_low_set;
		
	// Timer0 interrupt enabled
	ET0 = 1;
		
	// Enable Timer0
	TCON |= 0x10;
}

//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

void timer_isr (void) __interrupt (INTERRUPT_TIMER0)
{
	TH0 = timer0_high_set;
	TL0 = timer0_low_set;

	timer0_flag = 1;
	//printf("timer_isr invoked\r\n");
    
	if (buddy_ctx.daq_state == GENERAL_CTRL_ADC_ENABLE) {
		//P3 = P3 & ~0x40;
		//P3 = P3 | 0x40;
		
		if (adc_channel_index == (adc_channel_count - 1)) {
			AMX0P = adc_mux_tbl_n[0];
			AMX0N = adc_mux_tbl_p[0];
    }
    else {
      AMX0P = adc_mux_tbl_n[adc_channel_index + 1];
			AMX0N = adc_mux_tbl_p[adc_channel_index + 1];
		}
  } else if (buddy_ctx.daq_state == GENERAL_CTRL_COUNTER_ENABLE) {
		build_counter_packet();
  }
}