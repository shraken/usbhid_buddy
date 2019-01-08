#include "timers.h"

/// set whenever the stream timer has expired
uint8_t timer2_flag = 0;

/// high and low byte reload register values for the timer2.  These are
/// computed in the initial configuration and must be used to reload
/// timer2 in the timer2 interrupt.
static uint8_t timer2_low_set;
static uint8_t timer2_high_set;

/// chip timer assignments
// timer0 - i2c clock source
// timer1 - uart0
// timer2 - stream mode interrupt
// timer3 - i2c timeout detection

/**
 * @brief initialize the core timers
 * 
 */
void timers_init(void)
{
    timer0_init();
    timer2_init();
    timer3_init();
}

/**
 * @brief initialize timer0 used for i2c clock source.
 * 
 */
void timer0_init(void)
{
// Make sure the Timer can produce the appropriate frequency in 8-bit mode
// Supported SMBus Frequencies range from 10kHz to 100kHz.  The CKCON register
// settings may need to change for frequencies outside this range.
#if ((BUDDY_SYSCLK/SMB_FREQUENCY/3) < 255)
   #define SCALE 1
      CKCON |= 0x04;                   // Timer0 clock source = SYSCLK
#elif ((BUDDY_SYSCLK/SMB_FREQUENCY/4/3) < 255)
   #define SCALE 4
      // Timer1 clock source = SYSCLK / 4
    
      CKCON &= ~(0x07);
      CKCON |= 0x01;              
#endif

   TMOD &= ~(0x07);
   TMOD |= 0x02;                        // Timer0 in 8-bit auto-reload mode

   // Timer0 configured to overflow at 1/3 the rate defined by SMB_FREQUENCY
   TH0 = -(BUDDY_SYSCLK/SMB_FREQUENCY/SCALE/3);

   TL0 = TH1;                          // Init Timer0

   TR0 = 1;                            // Timer0 enabled
}

/**
 * @brief initialize timer3 used for i2c timeout detection.
 * 
 */
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
	TMR3RL = -(BUDDY_SYSCLK/12/40);           
	TMR3 = TMR3RL;                      
              
	// Timer3 interrupt enable
	EIE1 |= 0x80;
	
	// Start Timer3
	TMR3CN |= 0x04;    
}

/** @brief Configures Timer0 with default count values with a 16-bit mode.  The timer
 *				 is by default set to use a SYSCLK/12 reference, timer0 interrupt is enabled,
 *				 and the timer is enabled.
 *  @return Void.
 */
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

/** @brief Sets the requested period (in nsec) for Timer 2.  A calculation is performed to
 *				 determine if the timer0 clock base needs to be modified from the default SYSCLK/12
 *				 reference and is modified if need be.  The timer is used for stream mode for triggering
 *				 a conversion on the request DAQ function.
 *  @param period the time period (in nsec) that the timer elapses at.
 *  @return Void.
 */
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
	
	if ((period < (((1.0 /BUDDY_SYSCLK) * 0xFFFF) / 1e-9)) &&
	    (period > (((1.0 /BUDDY_SYSCLK) * 0x0001) / 1e-9))) {
		timer_set = 0xFFFF - (period * 1e-9) * BUDDY_SYSCLK;
		timer2_high_set = ((timer_set & 0xFF00) >> 8);
		timer2_low_set = (timer_set & 0x00FF);
		debug(("Timer2 set region 1 with %u\r\n", timer_set));

        // SYSCLK is timebase
        // Timer2 low byte and Timer high byte clock sets both use SYSCLK not TMR2CN
        CKCON |= 0x30; 
	} else if ((period < (((1.0 /(BUDDY_SYSCLK/12)) * 0xFFFF) / 1e-9)) &&
	    (period > (((1.0 /(BUDDY_SYSCLK/12)) * 0x0001) / 1e-9))) {
		timer_set = 0xFFFF - (period * 1e-9) * (BUDDY_SYSCLK/12);
		timer2_high_set = ((timer_set & 0xFF00) >> 8);
		timer2_low_set = (timer_set & 0x00FF);
		debug(("Timer2 set region 3 with %u\r\n", timer_set));
            
        // TMR2CN, Timer 2 clock is SYSCLK / 12
        TMR2CN &= ~(0x01);
	} else {
		debug(("Timer2 region not settable\r\n"));
			
		// Timer0 uses SYSCLK / 48
		//CKCON |= 0x02;
			
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

/**
 * @brief timer2 interrupt.  this is the stream mode interrupt source.  
 * 
 */
void timer2_isr(void) interrupt 5
{
    TF2H = 0;
	timer2_flag = 1;
	
    //P3 = P3 & ~0x40;
	//P3 = P3 | 0x40;
    
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

/**
 * @brief Timer3.  I2C timeout detection
 * 
 */
void timer3_isr(void) interrupt 14
{
   SMB0CF &= ~0x80;                    // Disable SMBus
   SMB0CF |= 0x80;                     // Re-enable SMBus
   TMR3CN &= ~0x80;                    // Clear Timer3 interrupt-pending flag
   STA = 0;
   SMB_BUSY = 0;                       // Free SMBus
}