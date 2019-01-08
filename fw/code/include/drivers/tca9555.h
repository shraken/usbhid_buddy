/**
 * @file tca9555.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2018-04-29
 * @brief TI TCA9555 Driver Library
 *			
 */

#ifndef  _TCA9555_H_
#define  _TCA9555_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "i2c.h"
#include "init.h"

/**
 * @brief TCA9555 driver error codes
 */
typedef enum _TCA9555_ERROR_CODE {
	TCA9555_ERROR_CODE_OK = 0,
	TCA9555_ERROR_CODE_GENERAL_ERROR = -1,
	TCA9555_ERROR_CODE_BAD_MEMORY = -2,
    TCA9555_ERROR_INDEX_OUT_RANGE = -3,
} TCA9555_ERROR_CODE;

/**
 * @brief register addresses for TCA9555
 */
typedef enum _TCA9555_REGISTER {
	TCA9555_REGISTER_IN_PORT_0 = 0x00,
	TCA9555_REGISTER_IN_PORT_1 = 0x01,
	TCA9555_REGISTER_OUT_PORT_0 = 0x02,
	TCA9555_REGISTER_OUT_PORT_1 = 0x03,
	TCA9555_REGISTER_POL_INV_PORT_0 = 0x04,
	TCA9555_REGISTER_POL_INV_PORT_1 = 0x05,
	TCA9555_REGISTER_CFG_PORT_0 = 0x06,
	TCA9555_REGISTER_CFG_PORT_1 = 0x07
} TCA9555_REGISTER;

typedef enum _TCA9555_PORT {
    TCA9555_PORT_0 = 0,
    TCA9555_PORT_1,
} TCA9555_PORT;

typedef enum _TCA9555_PIN {
    TCA9555_PIN_0 = 0,
    TCA9555_PIN_1,
    TCA9555_PIN_2,
    TCA9555_PIN_3,
    TCA9555_PIN_4,
    TCA9555_PIN_5,
    TCA9555_PIN_6,
    TCA9555_PIN_7,
} TCA9555_PIN;

/**
 * @brief simple index channel enum.  The bits to set are
 *		  calculated from a bit shift of these values.
 */
typedef enum _TCA9555_CHANNEL {
	// port 0
	TCA9555_CHANNEL_0_0 = 0,
	TCA9555_CHANNEL_0_1,
	TCA9555_CHANNEL_0_2,
	TCA9555_CHANNEL_0_3,
	TCA9555_CHANNEL_0_4,
	TCA9555_CHANNEL_0_5,
	TCA9555_CHANNEL_0_6,
	TCA9555_CHANNEL_0_7,
	
	// port 1
	TCA9555_CHANNEL_1_0,
	TCA9555_CHANNEL_1_1,
	TCA9555_CHANNEL_1_2,
	TCA9555_CHANNEL_1_3,
	TCA9555_CHANNEL_1_4,
	TCA9555_CHANNEL_1_5,
	TCA9555_CHANNEL_1_6,
	TCA9555_CHANNEL_1_7,
} TCA9555_CHANNEL;

/**
 * @brief sets the port.pin type as a high-impedance input or
 *	      an output.
 */
typedef enum _TCA9555_PIN_STATE {
	TCA9555_PIN_STATE_OUT = 0x00,
	TCA9555_PIN_STATE_IN = 0x01,
} TCA9555_PIN_STATE;

/**
 * @brief sets the polarity state of the GPIO expander.  Polarity
 *			  inversion can be enabled or disabled.
 */
typedef enum _TCA9555_POL_INV_STATE {
    TCA9555_POL_INV_STATE_DISABLE = 0,
    TCA9555_POL_INV_STATE_ENABLE,
} TCA9555_POL_INV_STATE;

/**
 * @brief enumeration used when expressing low or high pin state.  
 */
typedef enum _TCA9555_PIN_VALUE {
    TCA9555_PIN_VALUE_LOW = 0x00,
    TCA9555_PIN_VALUE_HIGH = 0x01,
} TCA9555_PIN_VALUE;

#define TCA95555_I2C_ADDRESS 0x20

int8_t tca9555_init(void);
int8_t tca9555_set_port_direction(uint8_t port_num, uint8_t pin_num, uint8_t dir);
int8_t tca9555_set_port_polarity(uint8_t port_num, uint8_t pin_num, uint8_t pol);
int8_t tca9555_set_port_pin(uint8_t port_num, uint8_t pin_num, uint8_t value);

#endif /* _TCA9555_H_ */