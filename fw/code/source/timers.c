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
uint8_t timer2_flag = 0;

static uint8_t timer0_low_set;
static uint8_t timer0_high_set;

static uint8_t timer2_low_set;
static uint8_t timer2_high_set;

void timer0_init(void)
{
	// set to 0xF05F (4000) ticks at 0.25 usec timer period
	// for equivalent interrupt of 1000 usec.
	
	TH0 = DEFAULT_TIMER0_HIGH_PERIOD;
	TL0 = DEFAULT_TIMER0_LOW_PERIOD;
	
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

void timer1_init(void)
{
#if ((SYSCLK/SMB_FREQUENCY/3) < 255)
	#define SCALE 1
	// Timer1 clock source = SYSCLK
	CKCON |= 0x08;
#elif ((SYSCLK/SMB_FREQUENCY/4/3) < 255)
	#define SCALE 4
	// Timer1 clock source = SYSCLK / 4
    CKCON |= 0x01;
    CKCON &= ~0x0A;
#else
	#error "SMB_FREQUENCY is impossible with current SYSCLK definition"
#endif
	
	// Timer1 in 8-bit auto-reload mode
	TMOD &= ~(0x30);
    TMOD |= 0x20;                        
	
	// Timer1 configured to overflow at 1/3 the rate defined by SMB_FREQUENCY
	TH1 = -(SYSCLK/SMB_FREQUENCY/SCALE/3);
	TL1 = TH1;                          
	
	// Timer1 enabled
	TR1 = 1;                            
}

void timer2_init(void)
{
    
}

void timer3_init(void)
{
	// Timer3 configured for 16-bit auto-
    // reload, low-byte interrupt disabled
	TMR3CN = 0x00;

	// Timer3 uses SYSCLK/12
	CKCON &= ~0x40;

	// Timer3 configured to overflow after
	// ~25ms (for SMBus low timeout detect):
	// 1/.025 = 40
	
	// @todo: sysclk isn't 12Mhz, its 48Mhz fix.
	TMR3RL = -(SYSCLK/12/40);           
	TMR3 = TMR3RL;                      
              
	// Timer3 interrupt enable
	EIE1 |= 0x80;
	
	// Start Timer3
	TMR3CN |= 0x04;
}

void timer0_set_period(uint32_t period)
{
	uint16_t timer_set;

	// Disable Timer0
	TCON &= ~(0x10);
	
	// Timer0 interrupt disabled
	ET0 = 0;
	
	debug(("timer0_set_period, nsec period = %lu\r\n", period));
	
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

void timer0_isr(void) interrupt 1
{
    /*
	TH0 = timer0_high_set;
	TL0 = timer0_low_set;

	timer0_flag = 1;
	
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
  */
}

void timer2_isr(void) interrupt 5
{
    TMR2H = timer2_high_set;
    TMR2L = timer2_low_set;
    
    timer2_flag = 1;
    
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

void timer3_isr(void) interrupt 14
{
	// Disable SMBus
	SMB0CF &= ~0x80;
	
	// Re-enable SMBus
	SMB0CF |= 0x80;
	
	// Clear Timer3 interrupt-pending
    // flag
	TMR3CN &= ~0x80;                    
   
	// Free SMBus
	STA = 0;
	SMB_BUSY = 0;
}