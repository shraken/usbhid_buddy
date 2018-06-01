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

uint8_t timer2_flag = 0;

static uint8_t timer2_low_set;
static uint8_t timer2_high_set;

void timer2_init(void)
{
    // By default, configure Timer2 to be a SYSCLK/12 base
    //
    
    // clear Timer2 low and high byte clock selects
    CKCON &= ~(0x30);
    
    // TMR2CN, Timer 2 clock is SYSCLK / 12
    TMR2CN &= ~(0x01);
    
 	// set to 0xF05F (4000) ticks at 0.25 usec timer period
	// for equivalent interrupt of 1000 usec.
	
	TMR2RLH = DEFAULT_TIMER2_HIGH_PERIOD;
	TMR2RLL = DEFAULT_TIMER2_LOW_PERIOD;
	   
    // Timer 2 interrupt enabled
    ET2 = 1;
    
    // Timer2 enable
    TR2 = 1;
}

void timer2_set_period(uint32_t period)
{
	uint16_t timer_set;

	// Disable Timer2
	TR2 = 0;
    
	// Timer2 interrupt disabled
	ET2 = 0;
	
	debug(("timer2_set_period, nsec period = %lu\r\n", period));
	
	// bound period by lower value of 20.83 nsec (21 nsec)
	// and upper value of 65535000 nsec.
	
	if (period < TIMER2_LOW_PERIOD) {
		period = TIMER2_LOW_PERIOD;
	} else if (period > TIMER2_HIGH_PERIOD) {
		period = TIMER2_HIGH_PERIOD;
	}
	
    // clear Timer2 low and high byte clock selects
    CKCON &= ~(0x30);
		
	// set Timer0 clock base and 16-bit timer comparator value
	
	if ((period < (((1.0 /SYSCLK) * 0xFFFF) / 1e-9)) &&
	    (period > (((1.0 /SYSCLK) * 0x0001) / 1e-9))) {
		timer_set = 0xFFFF - (period * 1e-9) * SYSCLK;
		timer2_high_set = ((timer_set & 0xFF00) >> 8);
		timer2_low_set = (timer_set & 0x00FF);
		debug(("Timer2 set region 1 with %u\r\n", timer_set));

        // SYSCLK is timebase
        // Timer2 low byte and Timer high byte clock sets both use SYSCLK not TMR2CN
        CKCON |= 0x30; 
	} else if ((period < (((1.0 /(SYSCLK/12)) * 0xFFFF) / 1e-9)) &&
	    (period > (((1.0 /(SYSCLK/12)) * 0x0001) / 1e-9))) {
		timer_set = 0xFFFF - (period * 1e-9) * (SYSCLK/12);
		timer2_high_set = ((timer_set & 0xFF00) >> 8);
		timer2_low_set = (timer_set & 0x00FF);
		debug(("Timer2 set region 3 with %u\r\n", timer_set));
				 
		// Timer2 low and high byte clock selects us TMR2CN
		CKCON &= ~(0x30);
            
        // TMR2CN, Timer 2 clock is SYSCLK / 12
        TMR2CN &= ~(0x01);
	} else {
		debug(("Timer2 region not settable\r\n"));
			
		// Timer0 uses SYSCLK / 48
		CKCON |= 0x02;
			
		timer2_high_set = 0x00;
		timer2_low_set = 0x00;
	}
		
	TMR2RLH = timer2_high_set;
	TMR2RLL = timer2_low_set;
		
    TMR2 = TMR2RL;
		
	// Enable Timer2
	TR2 = 1;
    
    // Timer2 interrupt enabled
	ET2 = 1;
}

//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

void timer2_isr(void) interrupt 5
{
    /*
	TH0 = timer0_high_set;
	TL0 = timer0_low_set;
    */
    
    TF2H = 0;
	timer2_flag = 1;
	
    P3 = P3 & ~0x40;
	P3 = P3 | 0x40;
    
	if (buddy_ctx.daq_state == GENERAL_CTRL_ADC_ENABLE) {
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