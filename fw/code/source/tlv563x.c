/**
 * @file tlv563x.c
 * @author Nicholas Shrake
 * @date 5 May 2017
 * @brief Driver library for TI TLV563x SPI DAC.
 *
 */

#include "drivers/tlv563x.h"

// shraken TODO: make this configurable as build directive
static uint8_t tlv563x_resolution = TLV5630_RESOLUTION_TYPE;

/// TLV563X SPI DAC chip CTRL0 and CTRL1 register values
/// consult pg. 11 of the TLV563x datasheet
static uint8_t tlv563x_ctrl0_reg = DEFAULT_TLV563X_CTRL0_REG;
static uint8_t tlv563x_ctrl1_reg = DEFAULT_TLV563X_CTRL1_REG;

/**
 * @brief Writes the word reg_value into the register defined by the
 *			reg_channel parameter.
 *
 * @param reg_channel register to be written into, see @TLV5630_REGISTERS
 * @param reg_value 16-bit value to be written to the selected register
 * @return Void.
 */
void tlv563x_write(uint8_t reg_channel, uint16_t reg_value)
{
	spi_select();
	
	spi_data_tx[0] = (reg_channel << 4);
	spi_data_tx[0] |= ((reg_value & 0x0F00) >> 8);
	spi_data_tx[1] = (reg_value & 0xFF);
	
	//debug(("TLV5630_write_block: writing %02bx:%02bx\r\n", SPI_Data_Array[0], SPI_Data_Array[1]));
	spi_bytes_trans = 2;
	spi_array_readwrite();

	return;
}

/**
 * @brief Configures the TLV563x DAC setting the CTRL0 and CTRL1 register
 * 			values.
 *
 * @return Void.
 */
void tlv563x_dac_init(void)
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
	
	#if 0
	// set reference select to external
	tlv563x_ctrl0_reg |= (1 << TLV563X_CTRL0_REFERENCE_0);
	
    // CTRL0
	tlv563x_reg_write(TLV563X_REG_CTRL0, tlv563x_ctrl0_reg);
	// Timer0_wait(1);
    
    // CTRL1
	tlv563x_reg_write(TLV563X_REG_CTRL1, tlv563x_ctrl1_reg);
    // Timer0_wait(1);
	
	tlv563x_dac_reset();
	#endif
	
	return;
}

/**
 * @brief Set the TLV563x device in a power mode OFF state.
 * @return Void.
 */
void tlv563x_disable(void) {
	tlv563x_ctrl0_reg |= (1 << TLV563X_CTRL0_POWER);
	tlv563x_reg_write(TLV563X_REG_CTRL0, tlv563x_ctrl0_reg);
}

/**
 * @brief Set the TLV563x device in a power mode ON state.
 * @return Void.
 */
void tlv563x_enable(void) {
	tlv563x_ctrl0_reg &= ~(1 << TLV563X_CTRL0_POWER);
	tlv563x_reg_write(TLV563X_REG_CTRL0, tlv563x_ctrl0_reg);
}

/**
 * @brief Helper function for single reg write interface.
 * @param reg register 
 * @return Void.
 */
void tlv563x_reg_write(uint8_t reg, uint8_t value) {
	tlv563x_write(reg, value);
}

/**
 * @brief Reset all DAC values on the device to zero.  Usually called
 *			by initialization routine when device is first brought up.
 *
 * @return Void.
 */
void tlv563x_dac_reset(void)
{
	int i;
	
	// set all DAC channels to zero outputs
	for (i = TLV563X_REG_DAC_A; i <= TLV563X_REG_DAC_H; i++) {
		tlv563x_write(i, 0);  
  }
}