/**
 * @file counter.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-11-24
 * @brief Tick counter using level or edge based interrupt
 *				 with the INT0/INT1 Configuration of the C8051
 *				 chip.
 *			
 */

#ifndef  _COUNTER_H
#define  _COUNTER_H

#include <buddy.h>
#include <stdint.h>
#include <utility.h>

#define RUNTIME_DEFAULT_COUNTER_CONTROL \
	((RUNTIME_COUNTER_CONTROL_ACTIVE_HIGH << RUNTIME_COUNTER_CONTROL_ACTIVE_BITPOS) | \
	(RUNTIME_COUNTER_CONTROL_TRIGGER_EDGE << RUNTIME_COUNTER_CONTROL_TRIGGER_BITPOS))

#define COUNTER_CHANNEL_0 0
#define COUNTER_CHANNEL_1 1

#define COUNTER_ITEM_SIZE sizeof(uint32_t)

/**
 * \enum COUNTER_ERROR_CODE
 * \brief list of error codes that can be returned by counter functions.
 */
typedef enum _COUNTER_ERROR_CODE {
	COUNTER_ERROR_CODE_OK = 0,
	COUNTER_ERROR_CODE_GENERAL_ERROR = -1,
} COUNTER_ERROR_CODE;

/** @brief Sets internal state values for counter mode operation.  Enables tick counters
 *				 interrupts for the two channels and resets the counter tick channel values.
 *  @param control enum of type RUNTIME_COUNTER_CONTROL_ACTIVE specifying if the tick
 *				 should occur on high-to-low or low-to-high transition.
 *  @param chan_mask enum of BUDDY_CHANNELS_MASK bitmask values representing the channels
 *				 requested for operation.
 *  @return COUNTER_ERROR_CODE_OK on success, COUNTER_ERROR_CODE_GENERAL_ERROR on error.
 */
int8_t counter_init(uint8_t control, uint8_t chan_mask);

/** @brief Resets the tick counter values for both channel0 and channel1.  Typically called
 *				 during the init function.
 *  @return Void.
 */
void counter_reset(void);

/** @brief Configures pins for counter operation by setting them as digital inputs.
 *  @return Void.
 */
void counter_pin_init(void);

/** @brief Enable counter mode by enabling tick counter interrupt for the
 *				 channel provided in the counter_init function call.
 *  @return Void.
 */
void counter_enable(void);

/** @brief Disable counter mode by disabling tick counter interrupt for the
 *				 channel provided in the counter_init function call.
 *  @return Void.
 */
void counter_disable(void);

/** @brief Get the tick counts for channel0 tick counter.
 *  @return The number of ticks recorded on the channel0 counter input.
 */
int32_t counter_get_chan0(void);

/** @brief Get the tick counts for channel1 tick counter.
 *  @return The number of ticks recorded on the channel1 counter input.
 */
int32_t counter_get_chan1(void);

void int0_isr (void) __interrupt (INTERRUPT_INT0);
void int1_isr (void) __interrupt (INTERRUPT_INT1);

#endif