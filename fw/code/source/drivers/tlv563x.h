#ifndef  _TLV563X_H_
#define  _TLV563X_H_

#include <stdint.h>

#define TLV5630_CHANNEL_COUNT 8        // TLV5630 Number of Channels                            // Master an error occurred

// informs the bit resolution of the TLV563x DAC
typedef enum _TLV563x_TYPE {
		TLV5632_TYPE = 8,		// 8bits
		TLV5631_TYPE = 10,	// 10bits
		TLV5630_TYPE = 12,	// 12bits
} TLV563x_TYPE;

// CTRL0 register bits, pg. 12
typedef enum _TLV5630_CTRL0_BITMASK {
    CTRL0_INPUT_MODE = 0,    // IM, 0 = straight binary, 1 = twos complement
    CTRL0_REFERENCE_0,       // R0, 01 = external, 02 = internal 1V, 03 = internal 2V
    CTRL0_REFERENCE_1,       // R1
    CTRL0_DOUT,              // DOUT, 0 = SPI DOUT disable, 1 = enabled
    CTRL0_POWER,             // Full Power Down, 0 = Power Down disabled, 1 = Power Down enabled
} TLV5630_CTRL0_BITMASK;

// CTRL1 register bits, pg. 12
typedef enum _TLV5630_CTRL1_BITMASK {
    CTRL1_SPEED_DAC_AB = 0,  // Speed AB, 1 = fast, 0 = slow
    CTRL1_SPEED_DAC_CD,      // Speed CD, 1 = fast, 0 = slow
    CTRL1_SPEED_DAC_EF,      // Speed EF, 1 = fast, 0 = slow
    CTRL1_SPEED_DAC_GH,      // Speed GH, 1 = fast, 0 = slow
    CTRL1_POWER_DAC_AB,      // Power Down Channel AB, 0 = disabled, 1 = enabled
    CTRL1_POWER_DAC_CD,      // Power Down Channel CD, 0 = disabled, 1 = enabled
    CTRL1_POWER_DAC_EF,      // Power Down Channel EF, 0 = disabled, 1 = enabled
    CTRL1_POWER_DAC_GH,      // Power Down Channel GH, 0 = disabled, 1 = enabled
} TLV5630_CTRL1_BITMASK;

typedef enum _TLV5630_REF_MODE {
		TLV5630_REF_MODE_EXT = 0,
		TLV5630_REF_MODE_INT_1V = 2,
		TLV5630_REF_MODE_INT_2V = 3,
} TLV5630_REF_MODE;

typedef enum _TLV5630_PWR_MODE {
		TLV5630_PWR_MODE_OFF = 0,
		TLV5630_PWR_MODE_ON,
} TLV5630_PWR_MODE;

typedef enum _TLV5630_REGISTERS {
    REG_DAC_A = 0,
    REG_DAC_B,
    REG_DAC_C,
    REG_DAC_D,
    REG_DAC_E,
    REG_DAC_F,
    REG_DAC_G,
    REG_DAC_H,
    REG_CTRL0,
    REG_CTRL1,
    REG_PRESET,
    REG_RESV,
    REG_DAC_A_B_COMP,
    REG_DAC_C_D_COMP,
    REG_DAC_E_F_COMP,
    REG_DAC_G_H_COMP,
} TLV5630_REGISTERS;

void TLV5630_DAC_Reset(void);
void TLV5630_write(uint8_t reg_channel, uint16_t reg_value);
void TLV5630_DAC_Init(void);
void TLV5630_DAC_set_power_mode(uint8_t power_state);

#endif