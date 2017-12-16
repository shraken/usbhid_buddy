/** @file globals.h
 *  @brief Global macros 
 */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdint.h>
#include <buddy.h>

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

// error code used globally for all routines
enum _config_error
{
    E_SUCCESS = 0,
    E_GENERAL_FAIL = -1,
    E_INDEX_OUT_BOUND = -2
};

typedef enum _config_error error_t;

//#define SYSCLK        12000000/1         // SYSCLK frequency in Hz

// USB clock selections (SFR CLKSEL)
#define USB_4X_CLOCK       0x00        // Select 4x clock multiplier, for USB
#define USB_INT_OSC_DIV_2  0x10        // Full Speed
#define USB_EXT_OSC        0x20
#define USB_EXT_OSC_DIV_2  0x30
#define USB_EXT_OSC_DIV_3  0x40
#define USB_EXT_OSC_DIV_4  0x50

// System clock selections (SFR CLKSEL)
#define SYS_INT_OSC        0x00        // Select to use internal oscillator
#define SYS_EXT_OSC        0x01        // Select to use an external oscillator
#define SYS_4X_DIV_2       0x02
#define SYS_4x             0x03

#define STATUS_TX_LED_PIN GPIO_P3_4
#define STATUS_RX_LED_PIN GPIO_P3_5
#define HEARTBEAT_PIN GPIO_P4_7
#define TLV563X_LDAC_PIN GPIO_P1_7

#endif