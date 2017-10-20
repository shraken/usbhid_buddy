#ifndef  _BUDDY_H_
#define  _BUDDY_H_

#include <stdint.h>
//#include <stdbool.h>

#if defined(C8051)
#include <globals.h>
#endif

// C8051 System Clock Frequency
#define SYSCLK     48000000

#define MAX_REPORT_SIZE 63
#define MAX_OUT_SIZE MAX_REPORT_SIZE
#define MAX_IN_SIZE MAX_REPORT_SIZE

#define NUM_DAC_CHANNELS 8

#define BUDDY_OUT_DATA_ID 0x01
#define BUDDY_IN_DATA_ID  0x02

#define BUDDY_TYPE_OFFSET 0        // USB HID (IN/OUT)
#define BUDDY_APP_CODE_OFFSET 1    // APP (Application)
#define BUDDY_APP_INDIC_OFFSET 2   // CTRL (Control) Type
#define BUDDY_APP_VALUE_OFFSET 3   // CTRL (Control) Value

#define DEFAULT_ADC0CN 0x01
#define DEFAULT_REF0CN 0x8F
#define DEFAULT_ADC0CF (((SYSCLK/8000000)-1)<<3)

#define BUDDY_BIT_SIZE 8
#define BUDDY_MAX_COUNTER 0x7F

//typedef int bool;
#define bool int
#define false 0
#define true 1

/**
 * \enum APP_CODE 
 * \brief action code specified by either the
 *		  host (ctrl, dac, pwm) or device (adc)
 *		  and occupies the first byte in the stream.
 */
typedef enum _APP_CODE {
	APP_CODE_CTRL = 0x00,
	APP_CODE_DAC,
	APP_CODE_PWM,
	APP_CODE_ADC,
	APP_CODE_TRIGGER,
	APP_CODE_INFO,
} APP_CODE;

/**
 * \enum GENERAL_CTRL 
 * \brief indicates the requested function of either
 *			DAC, ADC, PWM, or disabled.
 */
typedef enum _GENERAL_CTRL {
	GENERAL_CTRL_NONE = 0x00,
	GENERAL_CTRL_DAC_ENABLE,
	GENERAL_CTRL_PWM_ENABLE,
	GENERAL_CTRL_ADC_ENABLE,
	GENERAL_CTRL_LENGTH
} GENERAL_CTRL;

/**
 * \enum CTRL_CONTROL 
 * \brief control byte enumeration values.  the
 *		  control byte is the second byte sent in
 *		  the packet.  
 */
typedef enum _CTRL_CONTROL {
	CTRL_GENERAL = 0x00,
	CTRL_TIMING,
	CTRL_RUNTIME,
} CTRL_CONTROL;

/**
 * \enum MODE_CTRL 
 * \brief defines how the packets are processed.  Immediate
 *		  packets are processed as they arrive but stream will
 *		  buffer packets in a queue.  
 * @see ctrl_general_t
 */
typedef enum _MODE_CTRL {
	MODE_CTRL_IMMEDIATE = 0,
	MODE_CTRL_STREAM,
} MODE_CTRL;

/**
 * \enum RESOLUTION_CTRL 
 * \brief defines the resolution of the value communicated
 *		  in USB HID packet.
 * @see ctrl_general_t
 */
typedef enum _RESOLUTION_CTRL {
	RESOLUTION_CTRL_LOW = 0, 	// 8-bit
	RESOLUTION_CTRL_HIGH,	 	// 16-bit
	RESOLUTION_CTRL_SUPER,   	// 32-bit
} RESOLUTION_CTRL;

/**
 * \enum CODEC_STATUS
 * \brief codec internal status error codes.  
 * @see encode_packet
 * @see decode_packet
 */
typedef enum _CODEC_STATUS {
	CODEC_STATUS_CONTINUE = 2,
	CODEC_STATUS_FULL = 1,
	CODEC_STATUS_NOERR = 0,
	CODEC_STATUS_ERROR = -1,
} CODEC_STATUS;

/**
 * \enum RUNTIME_DAC_POWER 
 * \brief TLV563x power state
 * @see ctrl_runtime_t
 */
typedef enum _RUNTIME_DAC_POWER {
	RUNTIME_DAC_POWER_OFF = 0,
	RUNTIME_DAC_POWER_ON,
} RUNTIME_DAC_POWER;

/**
 * \enum RUNTIME_DAC_REF 
 * \brief TLV563x DAC reference voltage
 * @see ctrl_runtime_t
 */
typedef enum _RUNTIME_DAC_REF {
	RUNTIME_DAC_REF_EXT = 0,
	RUNTIME_DAC_REF_INT_1V,
	RUNTIME_DAC_REF_INT_2V,
} RUNTIME_DAC_REF;

/**
 * \enum RUNTIME_ADC_REF 
 * \brief C8051 ADC reference voltage
 * @see ctrl_runtime_t
 */
typedef enum _RUNTIME_ADC_REF {
	RUNTIME_ADC_REF_EXT = 0,
	RUNTIME_ADC_REF_INT,
	RUNTIME_ADC_REF_VDD,
} RUNTIME_ADC_REF;

/**
 * \enum RUNTIME_ADC_GAIN 
 * \brief C8051 ADC internal reference gain
 * @see ctrl_runtime_t
 */
typedef enum _RUNTIME_ADC_GAIN {
	RUNTIME_ADC_GAIN_1X = 0,
	RUNTIME_ADC_GAIN_2X,
} RUNTIME_ADC_GAIN;

/**
 * \enum RUNTIME_PWM_MODE 
 * \brief C8051 PWM mode (configuration)
 * @see ctrl_runtime_t
 */
typedef enum _RUNTIME_PWM_MODE {
	RUNTIME_PWM_MODE_FREQUENCY,
	RUNTIME_PWM_MODE_DUTY_CYCLE,
} RUNTIME_PWM_MODE;

/**
 * \enum RUNTIME_PWM_MODE 
 * \brief C8051 PWM mode (configuration)
 * @see ctrl_runtime_t
 */
typedef enum _RUNTIME_PWM_TIMEBASE {
	RUNTIME_PWM_TIMEBASE_SYSCLK = 0,
	RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_4 = 1,
	RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_12 = 2,
	RUNTIME_PWM_TIMEBASE_TIMER0_OVERFLOW = 3,
} RUNTIME_PWM_TIMEBASE;

/**
* \enum BUDDY_DATA_SIZE
* \brief 
*/
typedef enum _BUDDY_DATA_SIZE {
	BUDDY_DATA_SIZE_LOW = 1,		// 8-bit
	BUDDY_DATA_SIZE_HIGH = 2,		// 16-bit
	BUDDY_DATA_SIZE_SUPER = 4,	// 32-bit
} BUDDY_DATA_SIZE;

/**
 * \enum BUDDY_RESPONSE
 * \brief Header response field for USB HID IN packets
 *				 used to indicate if true packet is being sent
 *				 or filler to not stall the USB IN.
 */
typedef enum _BUDDY_RESPONSE {
	BUDDY_RESPONSE_FILLER = 0x00,
	BUDDY_RESPONSE_VALID = 0x80
} BUDDY_RESPONSE;

/**
 * \enum BUDDY_CHANNELS
 * \brief list of channel number sequentially indexed.
 */
typedef enum _BUDDY_CHANNELS {
	BUDDY_CHAN_0 = 0,
	BUDDY_CHAN_1,
	BUDDY_CHAN_2,
	BUDDY_CHAN_3,
	BUDDY_CHAN_4,
	BUDDY_CHAN_5,
	BUDDY_CHAN_6,
	BUDDY_CHAN_7,
  BUDDY_CHAN_LENGTH,
} BUDDY_CHANNELS;

/**
 * \enum BUDDY_CHANNELS_MASK
 * \brief bitmask used in setting the channel_mask field of the
 *		  ctrl_general_t structure.  Indicates what channels are
 *		  active and being requested for DAC or ADC operations.  
 */
typedef enum _BUDDY_CHANNELS_MASK {
	BUDDY_CHAN_0_MASK = (1 << BUDDY_CHAN_0),
	BUDDY_CHAN_1_MASK = (1 << BUDDY_CHAN_1),
	BUDDY_CHAN_2_MASK = (1 << BUDDY_CHAN_2),
	BUDDY_CHAN_3_MASK = (1 << BUDDY_CHAN_3),
	BUDDY_CHAN_4_MASK = (1 << BUDDY_CHAN_4),
	BUDDY_CHAN_5_MASK = (1 << BUDDY_CHAN_5),
	BUDDY_CHAN_6_MASK = (1 << BUDDY_CHAN_6),
	BUDDY_CHAN_7_MASK = (1 << BUDDY_CHAN_7),
} BUDDY_CHANNELS_MASK;

#ifdef SWIG
%constant int BUDDY_CHAN_ALL_MASK = (BUDDY_CHAN_0_MASK | 
							 		 BUDDY_CHAN_1_MASK | 
							 		 BUDDY_CHAN_2_MASK | 
							 		 BUDDY_CHAN_3_MASK |
							 		 BUDDY_CHAN_4_MASK | 
							 		 BUDDY_CHAN_5_MASK | 
							 		 BUDDY_CHAN_6_MASK | 
							 		 BUDDY_CHAN_7_MASK);
#else
#define BUDDY_CHAN_ALL_MASK (BUDDY_CHAN_0_MASK | \
							 BUDDY_CHAN_1_MASK | \
							 BUDDY_CHAN_2_MASK | \
							 BUDDY_CHAN_3_MASK | \
							 BUDDY_CHAN_4_MASK | \
							 BUDDY_CHAN_5_MASK | \
							 BUDDY_CHAN_6_MASK | \
							 BUDDY_CHAN_7_MASK)
#endif

/**
 * \enum CODEC_BIT_WIDTH
 * \brief codec bit widths of the ADC and DAC values communicated
 *		  through the internal packet.  Used by the codec to encode
 *		  and decode values between host and device.  
 */
typedef enum _CODEC_BIT_WIDTH {
	CODEC_BIT_WIDTH_2 = 2,
	CODEC_BIT_WIDTH_3 = 3,
	CODEC_BIT_WIDTH_4 = 4,
	CODEC_BIT_WIDTH_5 = 5,
	CODEC_BIT_WIDTH_6 = 6,
	CODEC_BIT_WIDTH_7 = 7,
	CODEC_BIT_WIDTH_8 = 8,
	CODEC_BIT_WIDTH_9 = 9,
	CODEC_BIT_WIDTH_10 = 10,
	CODEC_BIT_WIDTH_11 = 11,
	CODEC_BIT_WIDTH_12 = 12,
} CODEC_BIT_WIDTH;

/**
 * \enum FIRMWARE_INFO_DAC_TYPE
 * \brief Indicates the type of type of DAC device.  DAC devices have
 *		  different internal bit resolutions so incoming values must
 *		  be bit-shifted down/up.  
 */
typedef enum _FIRMWARE_INFO_DAC_TYPE {
	FIRMWARE_INFO_DAC_TYPE_NONE = 0x0,			// No DAC
	FIRMWARE_INFO_DAC_TYPE_TLV5630 = 0x01,		// TI TLV5630 12-bit
	FIRMWARE_INFO_DAC_TYPE_TLV5631 = 0x02,		// TI TLV5631 10-bit
	FIRMWARE_INFO_DAC_TYPE_TLV5632 = 0x03,		// TI TLV5632 8-bit
	FIRMWARE_INFO_DAC_TYPE_LENGTH
} FIRMWARE_INFO_DAC_TYPE;

/**
 * \struct firmware_info_t
 * \brief Firmware info structure that is retrieved by host application
 *			on boot to determine serial number, revision, and the
 *			DAC and memory types supported.
 */
typedef struct _firmware_info_t {
	uint32_t serial;
	uint32_t flash_datetime;
	uint8_t fw_rev_major;
	uint8_t fw_rev_minor;
	uint8_t fw_rev_tiny;
	uint8_t bootl_rev_major;
	uint8_t bootl_rev_minor;
	uint8_t bootl_rev_tiny;
	uint8_t type_dac;
} firmware_info_t;

/**
 * \struct ctrl_general_t
 * \brief ctrl_general_t structure used for basic configuration
 *				 of the buddy device.  Allows configuration of basic
 *				 mode (DAC, ADC, etc.), immediate/stream mode, oneshot/
 *				 continuous, saturate/wrap, channel mask, and bit
 *			   resolution.
 */
typedef struct _ctrl_general_t {
	uint8_t function;
	uint8_t mode;
	uint8_t channel_mask;
	uint8_t resolution;
} ctrl_general_t;

/**
 * \struct ctrl_runtime_t
 * \brief ctrl_runtime_t structure used for DAC, ADC, and PWM
 *					configuration settings.  
 */
typedef struct _ctrl_runtime_t {
	uint8_t dac_power;
	uint8_t dac_ref;
	uint8_t adc_ref;
	uint8_t adc_gain;
	uint8_t pwm_mode;
	uint8_t pwm_timebase;
} ctrl_runtime_t;

/**
 * \struct ctrl_timing_t
 * \brief ctrl_timing_t structure used for controlling Timer
 *				 interrupt frequency and ADC averaging.  The Timer
 *				 interrupt frequency dictates the sampling frequency
 *				 for both the DAC and ADC when operating in stream
 *				 mode.
 */
typedef struct _ctrl_timing_t {
	uint32_t period;
	uint8_t averaging;
} ctrl_timing_t;

/**
 * \struct general_packet_t
 * \brief General-purpose encoder packet type.  Channel values
 *				 represent either DAC or ADC values before they are
 *				 are encoded into a frame.
 */
typedef struct _general_packet_t {
	uint32_t channels[NUM_DAC_CHANNELS];
} general_packet_t;

#endif