#include <tca9555.h>
#include <i2c.h>

static uint8_t reg_out_0 = 0;
static uint8_t reg_out_1 = 0;

static uint8_t reg_polinv_0 = 0;
static uint8_t reg_polinv_1 = 0;

static uint8_t reg_cfg_0 = 0;
static uint8_t reg_cfg_1 = 0;

int8_t tca9555_init(void)
{
    int i;
    
	if (i2c_init(TCA95555_I2C_ADDRESS) != I2C_ERROR_CODE_OK) {
        return TCA9555_ERROR_CODE_GENERAL_ERROR;
    }
    
    // set port mode to output
    // no polarity inversion
    // set all outputs to low state
    
    for (i = 0; i < TCA9555_PIN_7; i++) {
        tca9555_set_port_direction(TCA9555_PORT_0, i, TCA9555_PIN_STATE_OUT);
        tca9555_set_port_direction(TCA9555_PORT_1, i, TCA9555_PIN_STATE_OUT);
    }

	return TCA9555_ERROR_CODE_OK;
}

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

    i2c_write(tbuf, 2);
	return TCA9555_ERROR_CODE_OK;
}

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

    i2c_write(tbuf, 2);
	return TCA9555_ERROR_CODE_OK;
}

int8_t tca9555_set_port_pin(uint8_t port_num, uint8_t pin_num, uint8_t value)
{
	return TCA9555_ERROR_CODE_OK;
}