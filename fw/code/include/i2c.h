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
#include <stdbool.h>
#include <string.h>
#include <compiler_defs.h>
#include <C8051F380_defs.h>
#include "globals.h"

extern bit SMB_BUSY;

#include <compiler_defs.h>
#include <c8051f380.h>

typedef enum _I2C_ERROR_CODE {
	I2C_ERROR_CODE_OK = 0,
	I2C_ERROR_CODE_GENERAL_ERROR = -1,
	I2C_ERROR_CODE_BAD_MEMORY = -2,
    I2C_ERROR_CODE_UNINITIALIZED = -3,
} I2C_ERROR_CODE;

#define  WRITE          0x00           // SMBus WRITE command
#define  READ           0x01           // SMBus READ command

#define  SMB_MTSTA      0xE0           // (MT) start transmitted
#define  SMB_MTDB       0xC0           // (MT) data byte transmitted
#define  SMB_MRDB       0x80           // (MR) data byte received

#define I2C_MAX_BUFFER_SIZE 16			// Maximum read/write buffer size for Master

#define I2C_SDA_PIN_NUM 0
#define I2C_SCL_PIN_NUM 1

int8_t i2c_write(uint8_t *buffer, uint16_t len);
int8_t i2c_init(uint8_t i2c_addr);

#endif /* _I2C_H_ */