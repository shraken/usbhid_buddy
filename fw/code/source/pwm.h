/**
 * @file pwm.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-09-26
 * @brief PWM support library for EFM8UB2 microcontroller.  Acts to
 *				 configure PCA module and allows setting the desired frequency
 *				 and duty cycle of the PCA modules.
 *			
 */

#ifndef _PWM_H
#define _PWM_H

#include <stdint.h>
#include <buddy.h>
#include <utility.h>

#define NUMBER_PCA_CHANNELS 5
#define DEFAULT_FREQUENCY 50000

/**
 * \enum PWM_ERROR_CODE
 * \brief list of error codes that can be returned by pwm functions.
 */
typedef enum _PWM_ERROR_CODE {
	PWM_ERROR_CODE_OK = 0,
	PWM_ERROR_CODE_GENERAL_ERROR = -1,
	PWM_ERROR_CODE_INDEX_ERROR = -2,
} PWM_ERROR_CODE;

/** @brief Initialize the PWM device.  Sets the pins to a PWM state and saves off
 *				 local PWM state values.  Checks if frequency or duty cycle operation is
 *				 requested and delegates to function.  
 *  @param mode enum of type RUNTIME_PWM_MODE specifying duty cycle or frequency mode.
 *  @param resolution enum of type BUDDY_DATA_SIZE specifying low (8-bit), high (16-bit),
 *				 or super (32-bit) resolution.
 *	@param chan_mask enum of BUDDY_CHANNELS_MASK bitmask values representing the channels
 *				 requested for operation.
 *  @return PWM_ERROR_CODE_OK on success, PWM_ERROR_CODE_GENERAL_ERROR on error.
 */
int8_t pwm_init(uint8_t mode, uint8_t resolution, uint8_t chan_mask);

/** @brief Configures pins for PWM operation crossbar mode with as a push/pull output. 
 *  @return Void.
 */
void pwm_pin_init(void);

/** @brief Enable PCA interrupts and enable PCA mode.
 *  @return Void.
 */
void pwm_enable(void);

/** @brief Disable PCA interrupts and disable PCA mode.
 *  @return Void.
 */
void pwm_disable(void);

/** @brief Sets the timebase used for PWM frequency mode of operation.  The timebase
 *				 specifies the maximum frequency that is a fraction of the provided PWM
 *				 counter value.
 *  @param value enum of type RUNTIME_PWM_TIMEBASE specifying the SYSCLK frequency used
 *				 as a timebase for PWM frequency mode.
 *  @return PWM_ERROR_CODE_OK on sucess, PWM_ERROR_CODE_GENERAL_ERROR on error.
 */
int8_t pwm_set_timebase(uint8_t value);

/** @brief Sets the PWM frequency for a given channel.
 *  @param channel enum of type BUDDY_CHANNELS specifying the channel to set frequency on.
 *  @param value integer value specifying the fractional count value that the pwm_timebase
 *				 should be set to.  The max integer value is 255, 65535, or 4294967295 depending
 *				 on the resolution setting specified in the pwm_init call.
 *  @return PWM_ERROR_CODE_OK on sucess, PWM_ERROR_CODE_GENERAL_ERROR on error.
 */
int8_t pwm_set_frequency(uint8_t channel, uint32_t value);

/** @brief Sets the PWM duty cycle for the given channel.
 *  @param channel enum of type BUDDY_CHANNELS specifying the channel to set duty cycle on.
 *  @param value integer value specifying the frational count value that the duty cycle is set
 *				 to.  The max integer value is 255, 65535, or 4294967295 depending
 *				 on the resolution setting specified in the pwm_init call.
 *
 *				 If the channel value is passed as 32767 and the resolution is set to 16-bit mode
 *				 then a 50% duty cycle will be set.
 *  @return PWM_ERROR_CODE_OK on sucess, PWM_ERROR_CODE_GENERAL_ERROR on error.
 */
int8_t pwm_set_duty_cycle(uint8_t channel, uint16_t value);

#endif