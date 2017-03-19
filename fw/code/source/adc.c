#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <adc.h>
#include <F3xx_USB0_InterruptServiceRoutine.h>
#include <F3xx_USB0_ReportHandler.h>
#include <c8051f3xx.h>
#include <utility.h>
#include <globals.h>
#include <gpio.h>

extern uint8_t daq_state;
extern uint8_t flag_usb_in;

extern uint8_t m_adc_control;
extern uint8_t m_adc_ref;
extern uint8_t m_adc_cfg;
			
uint8_t adc_complete = 0;
uint8_t adc_channel_index = 0;
uint8_t adc_channel_count = 0;
uint8_t adc_int_dec = 1;
uint8_t adc_int_dec_max = 1;
uint16_t adc_results[MAX_ANALOG_INPUTS];            

uint8_t adc_mux_tbl[MAX_ANALOG_INPUTS] = { 0 };

uint8_t adc_mux_ref_tbl[MAX_ANALOG_INPUTS] = {
		ADC_P2_7,	// ADC7_IN
		ADC_P2_6,	// ADC6_IN
		ADC_P2_5,	// ADC5_IN
		ADC_P2_4,	// ADC4_IN
		ADC_P2_3,	// ADC3_IN
		ADC_P2_2,	// ADC2_IN
		ADC_P2_1,	// ADC1_IN
		ADC_P2_0	// ADC0_IN		
};

void ADC0_Enable(void)
{
		// Enable ADC0
		AD0EN = 1;
}

void ADC0_Disable(void)
{
		// Disable ADC0
		AD0EN = 0;
}

void ADC0_Init(void)
{	
		AD0EN = 0;
		
		ADC0CN = DEFAULT_ADC0CN;
		REF0CN = DEFAULT_REF0CN;
		ADC0CF = DEFAULT_ADC0CF;
	
		AMX0P = adc_mux_tbl[0];
		AMX0N = 0x1F;
	
		EIE1 |= 0x08;
}

void ADC0_Set_Reference(uint8_t value)
{
		debug(("ADC0_Set_Reference(): value = %bd (%bx)\r\n", value, value));
		REF0CN = value;
}

void ADC0_ISR (void) interrupt 10
{
		//static uint8_t timer0_state = 0;
		static long adc_accumulator[MAX_ANALOG_INPUTS] = { 0 };
		int i;

		/*
		if (timer0_state)  {
				gpio_set_pin_value(TEST_STATUS_PIN, GPIO_VALUE_LOW);
				timer0_state = 0;
		} else {
				gpio_set_pin_value(TEST_STATUS_PIN, GPIO_VALUE_HIGH);
				timer0_state = 1;
		}
		*/

		// clear ADC interrupt
		AD0INT = 0;

		// if adc_complete flag has been set and not processed
		// then do not overwrite the temporary buffer
		/*
		if (adc_complete) {
				return;
		}
		*/

		adc_accumulator[adc_channel_index] += ADC0;
		if (adc_channel_index == (adc_channel_count - 1))
		{
				adc_int_dec--;

				if (adc_int_dec == 0) {
						adc_int_dec = adc_int_dec_max;

						for(i = 0; i < adc_channel_count; i++) {
								adc_results[i] = adc_accumulator[i] / adc_int_dec_max;
								adc_accumulator[i] = 0;
						}
				 
						adc_complete = 1;
						
						/*
						if (timer0_state)  {
								gpio_set_pin_value(TEST_STATUS_PIN, GPIO_VALUE_LOW);
								timer0_state = 0;
						} else {
								gpio_set_pin_value(TEST_STATUS_PIN, GPIO_VALUE_HIGH);
								timer0_state = 1;
						}
						*/
				}
				
				adc_channel_index = 0;
		} else {
				adc_channel_index++;
		}
}