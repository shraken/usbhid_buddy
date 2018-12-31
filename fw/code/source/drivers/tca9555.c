#include "tca9555.h"

static uint8_t reg_out_0 = 0;
static uint8_t reg_out_1 = 0;

static uint8_t reg_polinv_0 = 0;
static uint8_t reg_polinv_1 = 0;

static uint8_t reg_cfg_0 = 0;
static uint8_t reg_cfg_1 = 0;

/** boolean, set to True when device initialization has run
			otherwise set to False.  Initialization must be run before
		  use.  
*/
static bool deviceInit = false;

/**
 * @brief Initialize the TCA9555 I2C device.
 * @return Return code specified by TCA9555_ERROR_CODE enum.
 */
int8_t tca9555_init(void)
{
    int i;
        
		if (deviceInit) {
			return TCA9555_ERROR_CODE_OK;
		}
		
		if (i2c_init(TCA95555_I2C_ADDRESS << 1) != I2C_ERROR_CODE_OK) {
        return TCA9555_ERROR_CODE_GENERAL_ERROR;
    }

    // set port mode to output
    // no polarity inversion
    for (i = 0; i < TCA9555_PIN_7; i++) {
        tca9555_set_port_direction(TCA9555_PORT_0, i, TCA9555_PIN_STATE_OUT);
        Delay();
        tca9555_set_port_direction(TCA9555_PORT_1, i, TCA9555_PIN_STATE_OUT);
        Delay();
    }

    // set all outputs to low state to make ADC mode the default so we don't have
    // -10/+10V rails hanging off the terminal jacks
    for (i = 0; i < TCA9555_PIN_7; i++) {
        tca9555_set_port_pin(TCA9555_PORT_0, i, TCA9555_PIN_VALUE_HIGH);
        tca9555_set_port_pin(TCA9555_PORT_1, i, TCA9555_PIN_VALUE_HIGH);
    }

		deviceInit = true;
		return TCA9555_ERROR_CODE_OK;
}

/**
 * @brief Set the TCA9555 port direction to either input or output.
 * @param port_num enum of type TCA9555_CHANNEL specifying the channel
 * @param enum of type TCA9555_PIN_STATE specifying if output or input operation is desired
 * @return Return code specified by TCA9555_ERROR_CODE enum.
 */
int8_t tca9555_set_port_direction(uint8_t port_num, uint8_t pin_num, uint8_t dir)
{
    uint8_t tbuf[2];
        
    if ((pin_num < TCA9555_PIN_0) || (pin_num > TCA9555_PIN_7)) {
        return TCA9555_ERROR_INDEX_OUT_RANGE;
    }
    
    if (port_num == TCA9555_PORT_0) {
        if (dir == TCA9555_PIN_STATE_OUT) {
            reg_cfg_0 &= ~(1 << pin_num);
        } else if (dir == TCA9555_PIN_STATE_IN) {
            reg_cfg_0 |= (1 << pin_num);
        } else {
            return TCA9555_ERROR_INDEX_OUT_RANGE;
        }

        tbuf[0] = TCA9555_REGISTER_CFG_PORT_0;
        tbuf[1] = reg_cfg_0;
    } else if (port_num == TCA9555_PORT_1) {
        if (dir == TCA9555_PIN_STATE_OUT) {
            reg_cfg_1 &= ~(1 << pin_num);
        } else if (dir == TCA9555_PIN_STATE_IN) {
            reg_cfg_1 |= (1 << pin_num);
        } else {
            return TCA9555_ERROR_INDEX_OUT_RANGE;
        }
        
        tbuf[0] = TCA9555_REGISTER_CFG_PORT_1;
        tbuf[1] = reg_cfg_1;
    } else {
        return TCA9555_ERROR_CODE_GENERAL_ERROR;
    }

		/*
    printf("tca9555_set_port_direction invoked\r\n");
    printf("reg_cfg_0 = %bx\r\n", reg_cfg_0);
    printf("reg_cfg_1 = %bx\r\n", reg_cfg_1);
		*/
		
    i2c_write(tbuf, 2);
	return TCA9555_ERROR_CODE_OK;
}

/**
 * @brief Set the TCA9555 port and pin combination to a polarity inversion mode. 
 * @param port_num enum of type TCA9555_CHANNEL specifying the channel
 * @Param pol integer of 0 or 1 to set polarity inversion on provided channel
 * @return Return code specified by TCA9555_ERROR_CODE enum.
 */
int8_t tca9555_set_port_polarity(uint8_t port_num, uint8_t pin_num, uint8_t pol)
{
    uint8_t tbuf[2];
    
    if ((pin_num < TCA9555_PIN_0) || (pin_num > TCA9555_PIN_7)) {
        return TCA9555_ERROR_INDEX_OUT_RANGE;
    }
    
	if (port_num == TCA9555_PORT_0) {
        if (pol == TCA9555_POL_INV_STATE_DISABLE) {
            reg_polinv_0 &= ~(1 << pin_num);
        } else if (pol == TCA9555_POL_INV_STATE_ENABLE) {
            reg_polinv_0 |= (1 << pin_num);
        } else {
            return TCA9555_ERROR_INDEX_OUT_RANGE;
        }

        tbuf[0] = TCA9555_REGISTER_POL_INV_PORT_0;
        tbuf[1] = reg_polinv_0;
    } else if (port_num == TCA9555_PORT_1) {
        if (pol == TCA9555_POL_INV_STATE_DISABLE) {
            reg_polinv_1 &= ~(1 << pin_num);
        } else if (pol == TCA9555_POL_INV_STATE_ENABLE) {
            reg_polinv_1 |= (1 << pin_num);
        } else {
            return TCA9555_ERROR_INDEX_OUT_RANGE;
        }
        
        tbuf[0] = TCA9555_REGISTER_POL_INV_PORT_1;
        tbuf[1] = reg_polinv_1;
    } else {
        return TCA9555_ERROR_CODE_GENERAL_ERROR;
    }

		/*
    printf("tca9555_set_port_polarity invoked\r\n");
    printf("reg_cfg_0 = %bx\r\n", reg_cfg_0);
    printf("reg_cfg_1 = %bx\r\n", reg_cfg_1);
		*/
		
    i2c_write(tbuf, 2);
	return TCA9555_ERROR_CODE_OK;
}

/**
 * @brief Set the TCA9555 port and pin combination to a high or low state.
 * @param port_num enum of type TCA9555_CHANNEL specifying the channel
 * @Param value integer of 0 or 1 to set channel state low or high respectively
 * @return Return code specified by TCA9555_ERROR_CODE enum.
 */
int8_t tca9555_set_port_pin(uint8_t port_num, uint8_t pin_num, uint8_t value)
{
    uint8_t tbuf[2];
    
    if ((pin_num < TCA9555_PIN_0) || (pin_num > TCA9555_PIN_7)) {
        return TCA9555_ERROR_INDEX_OUT_RANGE;
    }
    
    if (port_num == TCA9555_PORT_0) {
        if (value == TCA9555_PIN_VALUE_LOW) {
            reg_out_0 &= ~(1 << pin_num);
        } else if (value == TCA9555_PIN_VALUE_HIGH) {
            reg_out_0 |= (1 << pin_num);
        } else {
            return TCA9555_ERROR_INDEX_OUT_RANGE;
        }

        tbuf[0] = TCA9555_REGISTER_OUT_PORT_0;
        tbuf[1] = reg_out_0;
    } else if (port_num == TCA9555_PORT_1) {
        if (value == TCA9555_PIN_VALUE_LOW) {
            reg_out_1 &= ~(1 << pin_num);
        } else if (value == TCA9555_PIN_VALUE_HIGH) {
            reg_out_1 |= (1 << pin_num);
        } else {
            return TCA9555_ERROR_INDEX_OUT_RANGE;
        }
        
        tbuf[0] = TCA9555_REGISTER_OUT_PORT_1;
        tbuf[1] = reg_out_1;
    } else {
        return TCA9555_ERROR_CODE_GENERAL_ERROR;
    }

		/*
    printf("tca9555_set_port_direction invoked\r\n");
    printf("reg_out_0 = %bx\r\n", reg_out_0);
    printf("reg_out_1 = %bx\r\n", reg_out_1);
    
    printf("tbuf[0] = %bx\r\n", tbuf[0]);
    printf("tbuf[1] = %bx\r\n", tbuf[1]);
		*/
		
    i2c_write(tbuf, 2);
	return TCA9555_ERROR_CODE_OK;
}