#include "adc.h"

/// channel index into the adc_results storage array.  This index is
/// advanced after each ADC channel is measured and the mux switched
/// to the next channel.
uint8_t __data adc_channel_index = 0;

/// the max number of ADC channels to be sampled
uint8_t adc_channel_count = 0;

/// running counter that specifies the averaging iteration for the
/// ADC input channel
uint8_t adc_int_dec = 1;

/// the number of iterations of a ADC channel value we accumulate
uint8_t adc_int_dec_max = 1;

/// measured ADC count array for the different channel inputs
int16_t __data adc_results[MAX_ANALOG_INPUTS];            

/// table of AMX0 negative channel register values for
/// corresponding channel index as specified by the array
/// index
uint8_t adc_mux_tbl_n[MAX_ANALOG_INPUTS] = { 0 };

/// table of AMX0 positive channel register values for
/// corresponding channel index as specified by the array
/// index
uint8_t adc_mux_tbl_p[MAX_ANALOG_INPUTS] = { 0 };

/// counter variable used to test ADC data path.  This counter is set
/// to ADC channels and observed on the host side
uint16_t adc_timer_count;

/// reference table of ADC0 mux register values.  These values are
/// the ADC mux register value that must be used to measure the ADC value
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
 */
void adc_enable(void) {
	AD0EN = 1;
}

/** @brief Disable the ADC.
 */
void adc_disable(void) {
	AD0EN = 0;
}

/** @brief Disables ADC, sets a default single ended conversion, sets up default ADC conversion
 *				 register values, and enables the ADC interrupt.
 */
void adc_init(void) {	
	AD0EN = 0;
		
	ADC0CN = DEFAULT_ADC0CN;
	REF0CN = DEFAULT_REF0CN;
	ADC0CF = DEFAULT_ADC0CF;
	
	AMX0P = adc_mux_tbl_n[0];
	AMX0N = ADC_GND;
	
	EIE1 |= 0x08;
}

/** @brief Sets the reference voltage used for the ADC.  The value is controlled by
 *				 the host driver and can be VDD, bandgap, or external reference.  
 *  @param register reference REF0CN value to be set
 */
void adc_set_reference(uint8_t value) {
	debug(("adc_set_reference(): value = %bd (%bx)\r\n", value, value));
	REF0CN |= value;
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