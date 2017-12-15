#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <buddy.h>

//#define ADC_TEST

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
} buddy_ctx_t;

/*
typedef struct _adc_ctx {
	
} adc_ctx;
*/

#define STATUS_TX_LED_PIN GPIO_P3_4
#define STATUS_RX_LED_PIN GPIO_P3_5
#define HEARTBEAT_PIN GPIO_P4_7
#define TLV563X_LDAC_PIN GPIO_P1_7

#endif