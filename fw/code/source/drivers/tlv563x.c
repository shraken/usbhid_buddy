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
#include <c8051f3xx.h>
#include <buddy.h>
#include <tlv563x.h>
#include <utility.h>

// shraken TODO: make this configurable as build directive
static uint8_t tlv563x_resolution = TLV5630_RESOLUTION_TYPE;

void tlv563x_write(uint8_t reg_channel, uint16_t reg_value)
{
	spi_select();
	
	SPI_Data_Tx_Array[0] = (reg_channel << 4);
	SPI_Data_Tx_Array[0] |= ((reg_value & 0x0F00) >> 8);
	SPI_Data_Tx_Array[1] = (reg_value & 0xFF);
	
	//debug(("TLV5630_write_block: writing %02bx:%02bx\r\n", SPI_Data_Array[0], SPI_Data_Array[1]));
	bytes_trans = 2;
	spi_array_readwrite();

	return;
}

void tlv563x_init(void)
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
	tlv563x_write(TLV563X_REG_CTRL0, 0x00);
    // Timer0_wait(1);
    
    // CTRL1
	tlv563x_write(TLV563X_REG_CTRL1, 0x00);
    // Timer0_wait(1);
	
	tlv563x_dac_reset();
	
	return;
}

void tlv563x_dac_reset(void)
{
	int i;
	
	// set all DAC channels to zero outputs
	for (i = TLV563X_REG_DAC_A; i <= TLV563X_REG_DAC_H; i++) {
		tlv563x_write(i, 0);  
  }
}

void tlv563x_dac_set_power_mode(uint8_t power_state)
{
	// CTRL0
	if (power_state) {
		tlv563x_write(TLV563X_REG_CTRL0, 0x00);
	} else {
		tlv563x_write(TLV563X_REG_CTRL0, 0x10);
	}
}