/**
 * @file poncho.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2018-09-24
 * @brief Wiggle Labs Poncho Expander Board Driver
 *			
 */

#ifndef  _PONCHO_H_
#define  _PONCHO_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <poncho.h>

/**
 * @brief Poncho Expander driver error codes
 */
typedef enum _PONCHO_ERROR_CODE {
	PONCHO_ERROR_CODE_OK = 0,
	PONCHO_ERROR_CODE_GENERAL_ERROR = -1,
} PONCHO_ERROR_CODE;

/**
 * \struct poncho_pin_cfg_t
 * \brief defines the pin mapping from equivalent buddy simple pins from the
 *        BUDDY_CHANNELS enum to the poncho board CtrlA and CtrlB PCA9555
 *        pin mux.  
 */
typedef struct _poncho_pin_cfg_t {
	uint8_t buddy_pin;     /* enum of BUDDY_CHANNELS defining the channel */
    uint8_t ctrl_a_pin;    /* the PCA9555 CTRL_A numbered pin */
    uint8_t ctrl_b_pin;    /* the PCA9555 CTRL_B numbered pin */
} poncho_pin_cfg_t;

/**
 * @brief Set the operating mode (IN or OUT) of the Poncho expander board.  The IN mode is
 *			  used for input signals (ADC & Counter).  The OUT mode is used for DAC and PWM output
 *			  applications.
 * @param pin enum of type BUDDY_CHANNELS specifying the channel index
 * @param pos enum of type TCA9555_PIN_VALUE specifying if a high or low pin state is requested
 * @return PONCHO_ERROR_CODE_OK on success, otherwise see PONCHO_ERROR_CODE.
 */
int8_t poncho_set_mode(uint8_t pin, uint8_t pos);

/**
 * @brief Set the requested buddy pin to an output mode.
 * @param pin enum of type BUDDY_CHANNELS specifying the channel index
 * @return PONCHO_ERROR_CODE_OK on success, otherwise see PONCHO_ERROR_CODE.
 */
int8_t poncho_set_out_mode(uint8_t pin);

/**
 * @brief Set the requested buddy pin to an output mode.
 * @param pin enum of type BUDDY_CHANNELS specifying the channel index
 * @return PONCHO_ERROR_CODE_OK on success, otherwise see PONCHO_ERROR_CODE.
 */
int8_t poncho_set_in_mode(uint8_t pin);

/**
 * @brief Put the poncho board in a default pin state.  The default pin state should
 *        make the IN mode active so that we never have big rail voltages accidentally
 *			  hanging off the terminal jack connectors.
 * @return PONCHO_ERROR_CODE_OK on success, otherwise see PONCHO_ERROR_CODE.
 */
int8_t poncho_default_config(void);

#endif /* _PONCHO_H_ */