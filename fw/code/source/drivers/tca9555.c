#include <tca9555.h>
#include <i2c.h>

int8_t tca9555_init(void)
{
	i2c_init();
	return TCA9555_ERROR_CODE_OK;
}

int8_t tca9555_set_port_direction(uint8_t port_num, uint8_t dir)
{
	return TCA9555_ERROR_CODE_OK;
}

int8_t tca9555_set_port_polarity(uint8_t port_num, uint8_t pol)
{
	return TCA9555_ERROR_CODE_OK;
}

int8_t tca9555_set_port_pin(uint8_t pin, uint8_t value)
{
	return TCA9555_ERROR_CODE_OK;
}