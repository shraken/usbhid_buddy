//#include <stdbool.h>
#include <usbhid_buddy.h>
#include "hidapi.h"

#define BUDDY_TEST_ADC_FREQ 1000		// 1 kHz
#define BUDDY_TEST_DAC_FREQ 2500		// 1 kHz

/** @brief test routine for sending DAC packets to the buddy
 *		instrument.  The channels, resolution, transfer type,
 *			and other runtime parameters are defined.
 *  @param hidapi handle pointedac_chan_valuer
 *	@param pointer to firmware info structure used to establish the device
 *			resolution.
 *	@param sample_rate the requested sample rate in hertz (Hz)
 *	@param streaming boolean type with true if streaming requested, otherwise immediate
 *  @param oneshot boolean type with true if oneshot, otherwise continuous
 *  @return -1 on failure, 0 on success.
 */
int8_t test_seq_dac(hid_device* handle, firmware_info_t *fw_info, 
				float sample_rate, bool streaming, bool oneshot);

/** @brief test routine for receiving ADC packets from the buddy
 *			instrument.  The channels, resolution, transfer type,
 *			and other runtime parameters are defined.
 *  @param hidapi handle pointer
 *	@param pointer to firmware info structure used to establish the device
 *			resolution.
 *	@param sample_rate the requested sample rate in hertz (Hz)
 *	@param streaming boolean type with true if streaming requested, otherwise immediate
 *  @param oneshot boolean type with true if oneshot, otherwise continuous
 *  @return -1 on failure, 0 on success.
 */
int8_t test_seq_adc(hid_device* handle, firmware_info_t *fw_info,
				float sample_rate, bool streaming, bool oneshot);