/**
 * @file tlv563x.c
 * @author Nicholas Shrake
 * @date 5 May 2017
 * @brief Driver library for TI TLV563x SPI DAC.
 *
 */

#include <spi.h>
#include <stdint.h>
#include <stdio.h>
#include <buddy.h>
#include <tlv563x.h>
#include <c8051f3xx.h>
#include <utility.h>

// shraken TODO: make this configurable as build directive
uint8_t xdata tlv563x_resolution = TLV5630_RESOLUTION_TYPE;

extern code firmware_info_t fw_info;

extern unsigned char xdata SPI_Data_Rx_Array[];
extern unsigned char xdata SPI_Data_Tx_Array[];
extern unsigned char xdata bytes_trans;

void TLV563x_write(uint8_t reg_channel, uint16_t reg_value)
{
	SPI_Select(SPI_DEVICE_TYPE_TLV563x);
	
	SPI_Data_Tx_Array[0] = (reg_channel << 4);
	SPI_Data_Tx_Array[0] |= ((reg_value & 0x0F00) >> 8);
	SPI_Data_Tx_Array[1] = (reg_value & 0xFF);
	
	//debug(("TLV5630_write_block: writing %02bx:%02bx\r\n", SPI_Data_Array[0], SPI_Data_Array[1]));
	bytes_trans = 2;
	SPI_Array_ReadWrite();

	return;
}

void TLV563x_DAC_Init(void)
{
	// set DAC bit resolution
	switch (fw_info.type_dac) {
		case FIRMWARE_INFO_DAC_TYPE_TLV5632:
			tlv563x_resolution = TLV5632_RESOLUTION_TYPE;
			break;
		
		case FIRMWARE_INFO_DAC_TYPE_TLV5631:
			tlv563x_resolution = TLV5631_RESOLUTION_TYPE;
			break;
		
		case FIRMWARE_INFO_DAC_TYPE_TLV5630:
		default:
			tlv563x_resolution = TLV5630_RESOLUTION_TYPE;
			break;
	}
	
    // CTRL0
	TLV563x_write(REG_CTRL0, 0x00);
    // Timer0_wait(1);
    
    // CTRL1
	TLV563x_write(REG_CTRL1, 0x00);
    // Timer0_wait(1);
	
	TLV563x_DAC_Reset();
	
	return;
}

void TLV563x_DAC_Reset(void)
{
	int xdata i;
	
	// set all DAC channels to zero outputs
	for (i = REG_DAC_A; i <= REG_DAC_H; i++) {
		TLV563x_write(i, 0);  
  }
}

void TLV563x_DAC_set_power_mode(uint8_t power_state)
{
	// CTRL0
	if (power_state) {
		TLV563x_write(REG_CTRL0, 0x00);
	} else {
		TLV563x_write(REG_CTRL0, 0x10);
	}
}