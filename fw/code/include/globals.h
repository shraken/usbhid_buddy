/** @file globals.h
 *  @brief Global macros 
 */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdint.h>
#include "buddy_common.h"

// I2C SCL clock frequency (in units of Hz)
#define SMB_FREQUENCY          100000

typedef struct _buddy_ctx_t {
	uint8_t daq_state;
	uint8_t m_ctrl_mode;
	uint8_t m_adc_mode;
	uint8_t m_pwm_mode;
	uint8_t m_pwm_timebase;
	uint8_t m_counter_control;
	uint8_t m_chan_mask;
	uint8_t m_resolution;
	uint8_t m_data_size;
	uint8_t m_chan_number;
	uint8_t m_chan_enable[BUDDY_CHAN_LENGTH];

    uint8_t m_expander_type;
	uint8_t m_expander_mode;
	uint8_t m_expander_pin_state;
} buddy_ctx_t;

#define STATUS_TX_LED_PIN GPIO_P3_4
#define STATUS_RX_LED_PIN GPIO_P3_5
#define HEARTBEAT_PIN GPIO_P4_7
#define TLV563X_LDAC_PIN GPIO_P1_7

#endif