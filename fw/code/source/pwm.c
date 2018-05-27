#include <stdio.h>
#include <stdint.h>
#include <C8051F3xx.h>
#include <pwm.h>

static uint32_t pwm_timebase = SYSCLK;
static uint32_t pwm_cex[NUMBER_PCA_CHANNELS];
static uint8_t pwm_chan_mask;
static uint8_t pwm_chan_enable[BUDDY_CHAN_LENGTH] = { 0 };
static uint8_t pwm_mode;
static uint8_t pwm_resolution = RESOLUTION_CTRL_HIGH;

void pwm_pin_init(void)
{
  // enable push-pull on P2.0 - P2.7
  P2MDOUT   = 0xFF;
	
  // crossbar skip over fixed P0, P1, and P2
  P0SKIP    = 0xCF;
  P1SKIP    = 0xF0;
  P2SKIP    = 0x00;  
	
  // disable high impedance on P2.0 - P2.7
  P2MDIN    = 0xFF;
	
  // enable PCA CEX0 - CEX4
  XBR0      = 0x03;
  XBR1      = 0x45;
}

int8_t pwm_duty_cycle_init(void)
{
	uint8_t i;
	
	// Stop counter; clear all flags
	PCA0CN = 0x00;
	
	// Use SYSCLK as time base
	PCA0MD = 0x08;      
	
	if (pwm_resolution == RESOLUTION_CTRL_LOW) {
		// 0100 1011
		// Module 0 = 8-bit PWM mode and 
		// enable Module 0 Match and Interrupt Flags
		//PCA0CPM0 = 0x4B;
		PCA0CPM0 = 0x42;
	} else if (pwm_resolution == RESOLUTION_CTRL_HIGH) {
		// 1100 1011
		// Module 0 = 16-bit PWM mode and 
		// enable Module 0 Match and Interrupt Flags
		PCA0CPM0 = 0xCB;
	} else {
		return PWM_ERROR_CODE_INDEX_ERROR;
	}
	
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (pwm_chan_enable[i]) {
			pwm_cex[i] = 0xFFFF;
			pwm_set_duty_cycle(i, pwm_cex[i]); 
		}
	}
				
	return PWM_ERROR_CODE_OK;
}

int8_t pwm_frequency_init(void)
{
	uint8_t i;

	// Stop counter; clear all flags
	PCA0CN = 0x00;

	// Use SYSCLK/12 as time base
	PCA0MD = 0x00;
	
	// 0100 0110
	// Module 0 = Frequency Output mode
  PCA0CPM0 = 0x46;
	
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (pwm_chan_enable[i]) {
			// Configure initial PWM frequency for 1 kHz
			pwm_cex[i] = DEFAULT_FREQUENCY;
			pwm_set_frequency(i, pwm_cex[i]);
		}
	}
	
	return PWM_ERROR_CODE_OK;
}

int8_t pwm_init(uint8_t mode, uint8_t resolution, uint8_t chan_mask)
{
	uint8_t i;

	pwm_pin_init();

	pwm_chan_mask = chan_mask;
	pwm_mode = mode;
	pwm_resolution = resolution;
	
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (chan_mask & (1 << i)) {
		  pwm_chan_enable[i] = 1;
		} else {
			pwm_chan_enable[i] = 0;
		}
	}
	
	if (mode == RUNTIME_PWM_MODE_FREQUENCY) {
		pwm_frequency_init();
	} else if (mode == RUNTIME_PWM_MODE_DUTY_CYCLE) {
	  pwm_duty_cycle_init();
	} else {
		return PWM_ERROR_CODE_GENERAL_ERROR;
	}

	return PWM_ERROR_CODE_OK;
}

void pwm_enable(void)
{
	// Enable PCA interrupts
  EIE1 |= 0x10;
	
	// Start PCA counter
  CR = 1;
}

void pwm_disable(void)
{
	// Stop counter; clear all flags
	PCA0CN = 0x00;
	
	// Enable PCA interrupts
  EIE1 &= ~(0x10);
	
	P2MDOUT   = 0x00;
	XBR1     &= ~(0x05);
}

int8_t pwm_set_timebase(uint8_t value)
{
		PCA0MD &= ~(0x0E);
	
		switch (value) {
			case RUNTIME_PWM_TIMEBASE_SYSCLK:
				PCA0MD |= 0x08;
				pwm_timebase = SYSCLK;
				break;
			
			case RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_4:
				PCA0MD |= 0x02;
				pwm_timebase = (SYSCLK / 4);
				break;
			
			case RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_12:
				PCA0MD |= 0x00;
				pwm_timebase = (SYSCLK / 12);
				break;
			
			case RUNTIME_PWM_TIMEBASE_TIMER0_OVERFLOW:
			default:
				return PWM_ERROR_CODE_INDEX_ERROR;
		}
		
		return PWM_ERROR_CODE_OK;
}

int8_t pwm_set_frequency(uint8_t channel, uint32_t value)
{
	uint8_t reg_value;
	
	if ((channel < BUDDY_CHAN_0) ||
		  (channel > BUDDY_CHAN_4)) {
	  return PWM_ERROR_CODE_INDEX_ERROR;
	}
	
	pwm_cex[channel] = value;
	
	reg_value = (pwm_timebase / (2 * pwm_cex[channel]));

	switch (channel) {
		case BUDDY_CHAN_0:
			PCA0CPH0 = reg_value;
			break;
					
		case BUDDY_CHAN_1:
			PCA0CPH1 = reg_value;
			break;
					
		case BUDDY_CHAN_2:
			PCA0CPH2 = reg_value;
			break;
					
		case BUDDY_CHAN_3:
			PCA0CPH3 = reg_value;
			break;
					
		case BUDDY_CHAN_4:
			PCA0CPH4 = reg_value;
			break;
					
		default:
			break;
	}

	return PWM_ERROR_CODE_OK;
}

int8_t pwm_set_duty_cycle(uint8_t channel, uint16_t value)
{
	if ((channel < BUDDY_CHAN_0) ||
		  (channel > BUDDY_CHAN_4)) {
	  return PWM_ERROR_CODE_INDEX_ERROR;
	}
	
	pwm_cex[channel] = value;
	
	switch (channel) {
		case BUDDY_CHAN_0:
			if (pwm_resolution == RESOLUTION_CTRL_LOW) {
					PCA0CPH0 = (pwm_cex[BUDDY_CHAN_0] & 0x00FF);
			} else if (pwm_resolution == RESOLUTION_CTRL_HIGH) {
					PCA0CPL0 = (pwm_cex[BUDDY_CHAN_0] & 0x00FF);
					PCA0CPH0 = (pwm_cex[BUDDY_CHAN_0] & 0xFF00)>>8;
			}
			break;
					
		case BUDDY_CHAN_1:
			if (pwm_resolution == RESOLUTION_CTRL_LOW) {
					PCA0CPH1 = (pwm_cex[BUDDY_CHAN_1] & 0x00FF);
			} else if (pwm_resolution == RESOLUTION_CTRL_HIGH) {
					PCA0CPL1 = (pwm_cex[BUDDY_CHAN_1] & 0x00FF);
					PCA0CPH1 = (pwm_cex[BUDDY_CHAN_1] & 0xFF00)>>8;
			}
			break;
					
		case BUDDY_CHAN_2:
			if (pwm_resolution == RESOLUTION_CTRL_LOW) {
					PCA0CPH2 = (pwm_cex[BUDDY_CHAN_2] & 0x00FF);
			} else if (pwm_resolution == RESOLUTION_CTRL_HIGH) {
					PCA0CPL2 = (pwm_cex[BUDDY_CHAN_2] & 0x00FF);
					PCA0CPH2 = (pwm_cex[BUDDY_CHAN_2] & 0xFF00)>>8;
			}
			break;
					
		case BUDDY_CHAN_3:
			if (pwm_resolution == RESOLUTION_CTRL_LOW) {
					PCA0CPH3 = (pwm_cex[BUDDY_CHAN_3] & 0x00FF);
			} else if (pwm_resolution == RESOLUTION_CTRL_HIGH) {
					PCA0CPL3 = (pwm_cex[BUDDY_CHAN_3] & 0x00FF);
					PCA0CPH3 = (pwm_cex[BUDDY_CHAN_3] & 0xFF00)>>8;
			}
			break;
					
		case BUDDY_CHAN_4:
			if (pwm_resolution == RESOLUTION_CTRL_LOW) {
					PCA0CPH4 = (pwm_cex[BUDDY_CHAN_4] & 0x00FF);
			} else if (pwm_resolution == RESOLUTION_CTRL_HIGH) {
					PCA0CPL4 = (pwm_cex[BUDDY_CHAN_4] & 0x00FF);
					PCA0CPH4 = (pwm_cex[BUDDY_CHAN_4] & 0xFF00)>>8;
			}
			break;
					
		default:
			break;
	}

	return PWM_ERROR_CODE_OK;
}

void PCA0_ISR (void) interrupt 11
{
	if (pwm_mode == RUNTIME_PWM_MODE_DUTY_CYCLE) {
		if (CCF0) {
			CCF0 = 0;
			
			if (pwm_resolution == RESOLUTION_CTRL_HIGH) {
				PCA0CPL0 = (pwm_cex[BUDDY_CHAN_0] & 0x00FF);
				PCA0CPH0 = (pwm_cex[BUDDY_CHAN_0] & 0xFF00)>>8;
			} else if (pwm_resolution == RESOLUTION_CTRL_LOW) {
				PCA0CPH0 = (pwm_cex[BUDDY_CHAN_0] & 0x00FF);
			}
		} else if (CCF1) {
			CCF1 = 0;
			
			if (pwm_resolution == RESOLUTION_CTRL_HIGH) {
				PCA0CPL1 = (pwm_cex[BUDDY_CHAN_1] & 0x00FF);
				PCA0CPH1 = (pwm_cex[BUDDY_CHAN_1] & 0xFF00)>>8;
			} else if (pwm_resolution == RESOLUTION_CTRL_LOW) {
				PCA0CPH1 = (pwm_cex[BUDDY_CHAN_1] & 0x00FF);
			}
		} else if (CCF2) {
			CCF2 = 0;
			
			if (pwm_resolution == RESOLUTION_CTRL_HIGH) {
				PCA0CPL2 = (pwm_cex[BUDDY_CHAN_2] & 0x00FF);
				PCA0CPH2 = (pwm_cex[BUDDY_CHAN_2] & 0xFF00)>>8;
			} else if (pwm_resolution == RESOLUTION_CTRL_LOW) {
				PCA0CPH2 = (pwm_cex[BUDDY_CHAN_2] & 0x00FF);
			}
		} else if (CCF3) {
			CCF3 = 0;
			
			if (pwm_resolution == RESOLUTION_CTRL_HIGH) {
				PCA0CPL3 = (pwm_cex[BUDDY_CHAN_3] & 0x00FF);
				PCA0CPH3 = (pwm_cex[BUDDY_CHAN_3] & 0xFF00)>>8;
			} else if (pwm_resolution == RESOLUTION_CTRL_LOW) {
				PCA0CPH3 = (pwm_cex[BUDDY_CHAN_3] & 0x00FF);
			}
		} else if (CCF4) {
			CCF4 = 0;
			
			if (pwm_resolution == RESOLUTION_CTRL_HIGH) {
				PCA0CPL4 = (pwm_cex[BUDDY_CHAN_4] & 0x00FF);
				PCA0CPH4 = (pwm_cex[BUDDY_CHAN_4] & 0xFF00)>>8;
			} else if (pwm_resolution == RESOLUTION_CTRL_LOW) {
				PCA0CPH4 = (pwm_cex[BUDDY_CHAN_4] & 0x00FF);
			}
		}
	}
}