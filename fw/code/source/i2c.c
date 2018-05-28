#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <i2c.h>
#include <globals.h>

#include <compiler_defs.h>
#include <C8051F380_defs.h>

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
uint8_t I2C_Rx_Array_Count = 0;
uint8_t I2C_Tx_Array_Count = 0;

uint8_t I2C_Data_Rx_Array[I2C_MAX_BUFFER_SIZE] = { 0 };
uint8_t I2C_Data_Tx_Array[I2C_MAX_BUFFER_SIZE] = { 0 };

bit SMB_BUSY;
bit SMB_RW;

static bool i2c_initialized = false;

int8_t i2c_write(uint8_t *buffer, uint16_t len) {
	uint8_t bytes_to_copy;
	
	if (!buffer) {
		return I2C_ERROR_CODE_BAD_MEMORY;
	}
	
	bytes_to_copy = (len >= I2C_MAX_BUFFER_SIZE) ? I2C_MAX_BUFFER_SIZE : len;
	
	while (SMB_BUSY);
	SMB_BUSY = 1;
	
	I2C_Tx_Array_Count = len;
	memcpy(I2C_Data_Tx_Array, buffer, bytes_to_copy);
	
	SMB_RW = 0;
	STA = 1;
	
	return I2C_ERROR_CODE_OK;
}

int8_t i2c_read(uint8_t *buffer, uint16_t len) {
	if (!buffer) {
		return I2C_ERROR_CODE_BAD_MEMORY;
	}
	
	I2C_Rx_Array_Count = (len >= I2C_MAX_BUFFER_SIZE) ? I2C_MAX_BUFFER_SIZE : len;
	
	while (SMB_BUSY);
	SMB_BUSY = 1;
	SMB_RW = 1;
	
	STA = 1;
	while (SMB_BUSY);
	
    // @todo copy i2c rx buffer to buffer
	memcpy(buffer, I2C_Data_Rx_Array, I2C_Rx_Array_Count);
	return I2C_ERROR_CODE_OK;
}

int8_t i2c_init(void) {
    if (!i2c_initialized) {
        i2c_initialized = true;
        SMB0CF = 0x5D;
    }
    
	return I2C_ERROR_CODE_OK;
}

void i2c_isr(void) interrupt 7
{
	return;
}