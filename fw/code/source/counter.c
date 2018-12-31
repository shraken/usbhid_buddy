#include "counter.h"

static uint8_t counter_chan_enable[BUDDY_CHAN_LENGTH] = { 0 };
static int32_t counter_int0_count = 0;
static int32_t counter_int1_count = 0;

/** @brief Get the tick counts for channel0 tick counter.
 *  @return The number of ticks recorded on the channel0 counter input.
 */
int32_t counter_get_chan0(void)
{
	return counter_int0_count;
}

/** @brief Get the tick counts for channel1 tick counter.
 *  @return The number of ticks recorded on the channel1 counter input.
 */
int32_t counter_get_chan1(void)
{
	return counter_int1_count;
}

/** @brief Sets internal state values for counter mode operation.  Enables tick counters
 *				 interrupts for the two channels and resets the counter tick channel values.
 *  @param control enum of type RUNTIME_COUNTER_CONTROL_ACTIVE specifying if the tick
 *				 should occur on high-to-low or low-to-high transition.
 *  @param chan_mask enum of BUDDY_CHANNELS_MASK bitmask values representing the channels
 *				 requested for operation.
 *  @return COUNTER_ERROR_CODE_OK on success, COUNTER_ERROR_CODE_GENERAL_ERROR on error.
 */
int8_t counter_init(uint8_t control, uint8_t chan_mask)
{
	uint8_t i;
	
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_1; i++) {
		if (chan_mask & (1 << i)) {
		  counter_chan_enable[i] = 1;
		} else {
			counter_chan_enable[i] = 0;
		}
	}
	
	TCON |= 0x05;
	IT01CF = 0x00;
	
	if (counter_chan_enable[COUNTER_CHANNEL_0]) {
		// enable CIO_0 as INT0 allocated to P0.1
		IT01CF &= ~(0x07);
		IT01CF |= 0x01;
	}
	
	if (counter_chan_enable[COUNTER_CHANNEL_1]) {
		// enable CIO_1 as INT1 allocated to P0.0
		IT01CF &= ~(0x70); 
		IT01CF |= 0x00;
	}
	
	if (control == RUNTIME_COUNTER_CONTROL_ACTIVE_HIGH) {
		// INT0 & INT1 active high
		IT01CF |= 0x88;
	} else {
		// INT0 & INT1 active low
		IT01CF &= ~(0x88);
	}
	
	counter_reset();
	
	return COUNTER_ERROR_CODE_OK;
}

/** @brief Configures pins for counter operation by setting them as digital inputs.
 *  @return Void.
 */
void counter_pin_init(void)
{
	P0SKIP = 0x03;
  P0MDOUT = P0MDOUT & ~(0x03);
}

/** @brief Resets the tick counter values for both channel0 and channel1.  Typically called
 *				 during the init function.
 *  @return Void.
 */
void counter_reset(void)
{
	// reset counters
	counter_int0_count = 0;
	counter_int1_count = 0;
}

/** @brief Enable counter mode by enabling tick counter interrupt for the
 *				 channel provided in the counter_init function call.
 *  @return Void.
 */
void counter_enable(void)
{
	if (counter_chan_enable[COUNTER_CHANNEL_0]) {
		EX0 = 1;
	}
	
	if (counter_chan_enable[COUNTER_CHANNEL_1]) {
		EX1 = 1;
	}
}

/** @brief Disable counter mode by disabling tick counter interrupt for the
 *				 channel provided in the counter_init function call.
 *  @return Void.
 */
void counter_disable(void)
{
	EX0 = 0;
	EX1 = 0;
}

/** @brief Pin change/level interrupt 0.  This interrupt gets triggered on
 * 	level transitions on the mapped interrupt 0 mapped pin.
 */
void INT0_ISR (void) interrupt 0
{
   counter_int0_count++;
}

/** @brief Pin change/level interrupt 1.  This interrupt gets triggered on
 * 	level transitions on the mapped interrupt 1 mapped pin.
 */
void INT1_ISR (void) interrupt 2
{
   counter_int1_count++;
}