/**
 * @file i2c.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2018-04-29
 * @brief I2C driver library
 *			
 */

#ifndef  _I2C_H_
#define  _I2C_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef enum _I2C_ERROR_CODE {
	I2C_ERROR_CODE_OK = 0,
	I2C_ERROR_CODE_GENERAL_ERROR = -1,
	I2C_ERROR_CODE_BAD_MEMORY = -2,
} I2C_ERROR_CODE;

#define I2C_MAX_BUFFER_SIZE 16			// Maximum read/write buffer size for Master

int8_t i2c_write(uint8_t *buffer, uint16_t len);
int8_t i2c_read(uint8_t *buffer, uint16_t len);
int8_t i2c_init(void);

#endif /* _I2C_H_ */