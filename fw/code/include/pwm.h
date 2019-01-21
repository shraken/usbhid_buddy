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
#include <stdio.h>
#include <stdint.h>
#include <C8051F3xx.h>
#include "utility.h"
#include "buddy_common.h"

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

int8_t pwm_init(uint8_t mode, uint8_t resolution, uint8_t chan_mask);
void pwm_pin_init(void);
void pwm_enable(void);
void pwm_disable(void);
int8_t pwm_set_timebase(uint8_t value);
int8_t pwm_set_frequency(uint8_t channel, uint32_t value);
int8_t pwm_set_duty_cycle(uint8_t channel, uint16_t value);

#endif