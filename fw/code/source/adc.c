#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <c8051f3xx.h>
#include <adc.h>
#include <utility.h>
#include <globals.h>
#include <gpio.h>
#include <process.h>
#include <io.h>

uint8_t __data adc_channel_index = 0;
uint8_t adc_channel_count = 0;
uint8_t adc_int_dec = 1;
uint8_t adc_int_dec_max = 1;
int16_t __data adc_results[MAX_ANALOG_INPUTS];            

uint8_t adc_mux_tbl_n[MAX_ANALOG_INPUTS] = { 0 };
uint8_t adc_mux_tbl_p[MAX_ANALOG_INPUTS] = { 0 };

uint16_t adc_timer_count;

uint8_t __code adc_mux_ref_tbl[MAX_ANALOG_INPUTS] = {
	ADC_P2_0,	// ADC0_IN
	ADC_P2_1,	// ADC1_IN
	ADC_P2_2,	// ADC2_IN
	ADC_P2_3,	// ADC3_IN
	ADC_P2_4,	// ADC4_IN
	ADC_P2_5,	// ADC5_IN
	ADC_P2_6,	// ADC6_IN
	ADC_P2_7,	// ADC7_IN
};

/** @brief Enable the ADC.
 *  @return 0 on sucess, -1 on error.
 */
int8_t adc_enable(void)
{
    debug(("adc_enable() invoked\r\n"));

	AD0EN = 1;

    debug(("adc_enable() exit\r\n"));

	return 0;
}

/** @brief Disable the ADC.
 *  @return 0 on sucess, -1 on error.
 */
int8_t adc_disable(void)
{
    debug(("\r\nadc_disable() invoked\r\n"));

	// Disable ADC0
	AD0EN = 0;
	
	return 0;
}

/** @brief Disables ADC, sets a default single ended conversion, sets up default ADC conversion
 *				 register values, and enables the ADC interrupt.
 *  @return 0 on sucess, -1 on error.
 */
int8_t adc_init(void)
{	
    debug(("adc_init() invoked\r\n"));

	AD0EN = 0;
		
	ADC0CN = DEFAULT_ADC0CN;
	REF0CN = DEFAULT_REF0CN;
	ADC0CF = DEFAULT_ADC0CF;
	
	AMX0P = adc_mux_tbl_n[0];
	AMX0N = ADC_GND;
	
	EIE1 |= 0x08;

    debug(("adc_init() exiting\r\n"));

	return 0;
}

/** @brief Sets the reference voltage used for the ADC.  The value is controlled by
 *				 the host driver and can be VDD, bandgap, or external reference.  
 *  @return 0 on sucess, -1 on error.
 */
int8_t adc_set_reference(uint8_t value)
{
	debug(("adc_set_reference(): value = %bd (%bx)\r\n", value, value));
	REF0CN |= value;
	
	return 0;
}

/** @brief ADC interrupt.  Save ADC channel value to accumulator storage.
 * 	When a sufficient number of ADC channel values are collected, an ADC
 * 	packet is built and encoded into the HID frame.
 */
void adc_isr (void) __interrupt (INTERRUPT_ADC0_EOC)
{
	static int32_t adc_accumulator[MAX_ANALOG_INPUTS] = { 0 };
	int i;

	//P3 = P3 & ~0x40;

    //printf("adc_isr invoked\r\n");
	// clear ADC interrupt
	AD0INT = 0;

	adc_accumulator[adc_channel_index] += ADC0;
	if (adc_channel_index == (adc_channel_count - 1))
	{
		adc_int_dec--;

		if (adc_int_dec == 0) {
			adc_int_dec = adc_int_dec_max;

			for(i = 0; i < adc_channel_count; i++) {
				adc_results[i] = (int16_t) (adc_accumulator[i] / adc_int_dec_max);
				adc_accumulator[i] = 0;
			}

			build_adc_packet();
		}
				
		adc_channel_index = 0;
	} else {
		adc_channel_index++;
	}
	
	//P3 = P3 | 0x40;
}