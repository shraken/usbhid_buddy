/**
 * @file tlv563x.h
 * @author Nicholas Shrake
 * @date 5 May 2017
 * @brief Driver enumerations and prototypes for the Texas
 *			Instruments TLV563x line of SPI controlled
 *			Digital to Analog Converters (DAC).  
 *
 */

#ifndef  _TLV563X_H_
#define  _TLV563X_H_

#include <stdint.h>

#define TLV563X_CHANNEL_COUNT 8        // TLV563x Number of Channels

extern code firmware_info_t fw_info;

extern uint8_t SPI_Data_Rx_Array[];
extern uint8_t SPI_Data_Tx_Array[];
extern uint8_t bytes_trans;

/**
 * @brief informs the bit resolution of TLV563x DAC
 */
typedef enum _TLV563x_RESOLUTION_TYPE {
		TLV5632_RESOLUTION_TYPE = 8,	// 8bits
		TLV5631_RESOLUTION_TYPE = 10,	// 10bits
		TLV5630_RESOLUTION_TYPE = 12,	// 12bits
} TLV563x_RESOLUTION_TYPE;
 
/**
 * @brief CTRL0 register bits, pg. 12 of TLV563x datasheet
 */
typedef enum _TLV563X_CTRL0 {
    TLV563X_CTRL0_INPUT_MODE = 0,    // IM, 0 = straight binary, 1 = twos complement
    TLV563X_CTRL0_REFERENCE_0,       // R0, 01 = external, 02 = internal 1V, 03 = internal 2V
    TLV563X_CTRL0_REFERENCE_1,       // R1
    TLV563X_CTRL0_DOUT,              // DOUT, 0 = SPI DOUT disable, 1 = enabled
    TLV563X_CTRL0_POWER,             // Full Power Down, 0 = Power Down disabled, 1 = Power Down enabled
} TLV563X_CTRL0;

/**
 * @brief CTRL1 register bits, pg. 12 of TLV563x datasheet
 */
typedef enum _TLV563X_CTRL1 {
    TLV563X_CTRL1_SPEED_DAC_AB = 0,  // Speed AB, 1 = fast, 0 = slow
    TLV563X_CTRL1_SPEED_DAC_CD,      // Speed CD, 1 = fast, 0 = slow
    TLV563X_CTRL1_SPEED_DAC_EF,      // Speed EF, 1 = fast, 0 = slow
    TLV563X_CTRL1_SPEED_DAC_GH,      // Speed GH, 1 = fast, 0 = slow
    TLV563X_CTRL1_POWER_DAC_AB,      // Power Down Channel AB, 0 = disabled, 1 = enabled
    TLV563X_CTRL1_POWER_DAC_CD,      // Power Down Channel CD, 0 = disabled, 1 = enabled
    TLV563X_CTRL1_POWER_DAC_EF,      // Power Down Channel EF, 0 = disabled, 1 = enabled
    TLV563X_CTRL1_POWER_DAC_GH,      // Power Down Channel GH, 0 = disabled, 1 = enabled
} TLV563X_CTRL1;

/**
 * @brief Reference select bits (R1 - R0) of CTRL0 register, pg. 12 of
 *			TLV563x datasheet.
 */
typedef enum _TLV563X_REF_MODE {
		TLV563X_REF_MODE_EXT = 0,
		TLV563X_REF_MODE_INT_1V = 2,
		TLV563X_REF_MODE_INT_2V = 3,
} TLV563X_REF_MODE;

/**
 * @brief Power mode bits (Pcy) of CTRL1 register, pg. 12 of TLV563x
 *			datasheet.
 */
typedef enum _TLV563X_PWR_MODE {
		TLV563X_PWR_MODE_OFF = 0,
		TLV563X_PWR_MODE_ON,
} TLV563X_PWR_MODE;

/**
 * @brief Register map used for configuration and setting DAC values.
 */
typedef enum _TLV563X_REG {
    TLV563X_REG_DAC_A = 0,
    TLV563X_REG_DAC_B,
    TLV563X_REG_DAC_C,
    TLV563X_REG_DAC_D,
    TLV563X_REG_DAC_E,
    TLV563X_REG_DAC_F,
    TLV563X_REG_DAC_G,
    TLV563X_REG_DAC_H,
    TLV563X_REG_CTRL0,
    TLV563X_REG_CTRL1,
    TLV563X_REG_PRESET,
    TLV563X_REG_RESV,
    TLV563X_REG_DAC_A_B_COMP,
    TLV563X_REG_DAC_C_D_COMP,
    TLV563X_REG_DAC_E_F_COMP,
    TLV563X_REG_DAC_G_H_COMP,
} TLV563X_REG;

/**
 * @brief Reset all DAC values on the device to zero.  Usually called
 *			by initialization routine when device is first brought up.
 *
 * @return Void.
 */
void tlv563x_dac_reset(void);

/**
 * @brief Writes the word reg_value into the register defined by the
 *			reg_channel parameter.
 *
 * @param reg_channel register to be written into, see @TLV5630_REGISTERS
 * @param reg_value 16-bit value to be written to the selected register
 * @return Void.
 */
void tlv563x_write(uint8_t reg_channel, uint16_t reg_value);

/**
 * @brief Configures the TLV563x DAC setting the CTRL0 and CTRL1 register
 * 			values.
 *
 * @return Void.
 */
void tlv563x_dac_init(void);

/**
 * @brief Sets the current global power state to turn ON/OFF the DAC device.  If
 *			power_state is TRUE then normal mode, if FALSE then power off device.
 *
 * @param power_state 1 to power on device, 0 to disable power
 * @return Void.
 */
void tlv563x_dac_set_power_mode(uint8_t power_state);

#endif /* _TLV563X_H_ */