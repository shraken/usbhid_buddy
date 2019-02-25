#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <utility.h>
#include <support.h>
#include <time.h>
#include <buddy.h>
#include <buddy_common.h>
#include <utility.h>
#include "codec.h"

/// workspace buffer used for sending HID OUT messages to the device
static uint8_t out_hold_buf[MAX_OUT_SIZE] = { 0 };

/// context defines configuration parameters
static buddy_driver_context driver_ctx = { 0 };

/// short look-up table to convert FIRMWARE_INFO_DAC_TYPE enum type indices
/// to equivalent C string name of the Buddy DAQ device
char *fw_info_dac_type_names[FIRMWARE_INFO_DAC_TYPE_LENGTH] = {
	"None",
	"TI TLV5630 (12-bit)",
	"TI TLV5631 (10-bit)",
	"TI TLV5632 (8-bit)",
};

/**
 * @brief Get the working buffer used in encode and decode operations.
 * 
 * @return uint8_t* the working buffer
 */
uint8_t *get_buffer(void) {
	return (uint8_t *) &out_hold_buf;
}

/**
 * @brief Get the context object
 * 
 * @return buddy_driver_context* 
 */
buddy_driver_context *buddy_get_context(void) {
	return &driver_ctx;
}

/** @brief open hidapi handle for Buddy VID/PID, set to non-blocking mode and
 *			return info on USB device and firmware.
 *	@param hid_info pointer to structure to store USB device information
 *  @return NULL on failure, hidapi handle pointer on success.
 */
hid_device* hidapi_init(buddy_hid_info_t *hid_info)
{
	hid_device *handle;
	wchar_t wstr[MAX_CHAR_LENGTH];
	int res;

	if (hid_init() == -1) {
		critical(("hidapi_init(): unable to init using hid_init()\n"))
		return NULL;
	}

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	handle = hid_open(BUDDY_USB_VID, BUDDY_USB_PID, NULL);
	if (!handle) {
		critical(("hidapi_init(): unable to open device\n"));
		return NULL;
    }
    
    printf("hidpai_init(): handle = %p\r\n", handle);

	// allocate hidinfo_t strings for USB device information
	hid_info->str_mfr = (char *) malloc(MAX_CHAR_LENGTH);
	hid_info->str_product = (char *) malloc(MAX_CHAR_LENGTH);
	hid_info->str_serial = (char *) malloc(MAX_CHAR_LENGTH);
	hid_info->str_index_1 = (char *) malloc(MAX_CHAR_LENGTH);

	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_CHAR_LENGTH);
	if (res >= 0) {
		wcstombs(hid_info->str_mfr, wstr, MAX_CHAR_LENGTH);
	} else {
		strcpy(hid_info->str_mfr, "");
	}

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_CHAR_LENGTH);
	if (res >= 0) {
		wcstombs(hid_info->str_product, wstr, MAX_CHAR_LENGTH);
	} else {
		strcpy(hid_info->str_product, "");
	}

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string(handle, wstr, MAX_CHAR_LENGTH);
	if (res >= 0) {
		wcstombs(hid_info->str_serial, wstr, MAX_CHAR_LENGTH);
	} else {
		strcpy(hid_info->str_serial, "");
	}

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = hid_get_indexed_string(handle, 1, wstr, MAX_CHAR_LENGTH);
	if (res >= 0) {
		wcstombs(hid_info->str_index_1, wstr, MAX_CHAR_LENGTH);
	} else {
		strcpy(hid_info->str_index_1, "");
	}

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, 1);

	return handle;
}

/** @brief sends a USB OUT request with the specified binary data
*   @param hidapi handle pointer
*   @param BUDDY_APP_CODE_OFFSET enum offset value
*   @param BUDDY_APP_INDIC_OFFSET enum offset value
*   @param binary data to be written in the USB HID OUT request
*   @param length of the binary data to written
*   @return BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
*/
int buddy_write_raw(hid_device *handle, uint8_t code, uint8_t indic, uint8_t *raw, uint8_t length)
{
	unsigned char out_buf[MAX_OUT_SIZE] = { 0 };
	uint8_t copy_length;

	out_buf[BUDDY_TYPE_OFFSET] = BUDDY_OUT_DATA_ID;
	out_buf[BUDDY_APP_CODE_OFFSET] = code;
	out_buf[BUDDY_APP_INDIC_OFFSET] = indic;

	copy_length = (length > (MAX_OUT_SIZE - 3)) ? (MAX_OUT_SIZE - 3) : length;

	if (raw) {
		memcpy(&out_buf[BUDDY_APP_VALUE_OFFSET], raw, copy_length);
	}
	
	if (buddy_write_packet(handle, &out_buf[0], MAX_OUT_SIZE) == BUDDY_ERROR_CODE_GENERAL) {
		return BUDDY_ERROR_CODE_GENERAL;
	}

	return BUDDY_ERROR_CODE_OK;
}

/**
 * @brief clear the USB HID read buffer
 * 
 * @param handle internal handle returned from buddy_init
 * @return int BUDDY_ERROR_CODE_OK on success, otherwise self-describing error code.
 */
int buddy_empty(hid_device *handle)
{
    uint8_t temp_buffer[MAX_IN_SIZE];
    int res;

    while (1) {
        res = buddy_read_packet(handle, temp_buffer, MAX_IN_SIZE);

        if (res == -1) {
            return BUDDY_ERROR_CODE_PROTOCOL; 
        } else if (res == 0) {
            break;
        }
    }

    return BUDDY_ERROR_CODE_OK;
}

/** @brief use the hidapi library to write a USBHID packet
 *  @param handle hidapi internal handle returned from buddy_init
 *  @param buffer pointer to OUT USBHID packet
 *  @param length number of the bytes specified by the pointer buffer
 *  @return -1 on failure, 0 on success. 
 */
int buddy_write_packet(hid_device *handle, unsigned char *buffer, int length)
{
	int res;

	res = hid_write(handle, buffer, length);
	if (res < 0) {
		critical(("buddy_write_packet: hid_write call failed, error = %d (%ls) handle = %p\n", 
			res, hid_error(handle), handle));
		return BUDDY_ERROR_CODE_GENERAL;
	}

	return BUDDY_ERROR_CODE_OK;
}

/** @brief use the hidapi library to read a USBHID packet
 *  @param handle hidapi internal handle returned from buddy_init
 *  @param buffer pointer to location to store the IN USBHID packet
 *  @param length number of bytes to read in
 *  @return -1 on failure, 0 on success. 
 */
int buddy_read_packet(hid_device *handle, unsigned char *buffer, int length)
{
	int res;

	res = hid_read(handle, buffer, length);
	return res;
}

int buddy_empty(hid_device *handle)
{
    uint8_t temp_buffer[MAX_IN_SIZE];
    int res;

    while (1) {
        res = buddy_read_packet(handle, temp_buffer, MAX_IN_SIZE);

        if (res == -1) {
            return BUDDY_ERROR_CODE_PROTOCOL; 
        } else if (res == 0) {
            break;
        }
    }

    return BUDDY_ERROR_CODE_OK;
}

/** @brief 
*   @param hidapi handle pointer
*   @param pointer to general_packet_t structure with DAC values to be sent
*	@param boolean indicating if stream mode is MODE_CTRL_STREAM or MODE_CTRL_IMMEDIATE
*   @param type enum of type APP_CODE specific to the send operation requested
*   @return BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
*/
int buddy_send_generic(hid_device *handle, general_packet_t *packet, bool streaming, uint8_t type)
{
	int err_code;
	err_code = codec_encode(out_hold_buf, packet);

	if ((!streaming) || (err_code == CODEC_STATUS_FULL)) {
        out_hold_buf[BUDDY_TYPE_OFFSET] = BUDDY_OUT_DATA_ID;
		out_hold_buf[BUDDY_APP_CODE_OFFSET] = type;
        out_hold_buf[BUDDY_APP_INDIC_OFFSET] = codec_get_encode_count();

		//printf("buddy_send_pwm() with encode_count = %d\n", encode_count);
		if (buddy_write_packet(handle, &out_hold_buf[0], MAX_OUT_SIZE) == BUDDY_ERROR_CODE_GENERAL) {
			critical(("buddy_send_pwm: buddy_write_packet call failed\n"));
			return BUDDY_ERROR_CODE_GENERAL;
		}

        codec_set_encode_count(0);
        codec_set_offset_count(0);

		return BUDDY_ERROR_CODE_OK;
	} else if (err_code == CODEC_STATUS_CONTINUE) {
		return BUDDY_ERROR_CODE_OK;
	} else {
		printf("buddy_send_generic: err_code = BUDDY_ERROR_CODE_GENERAL\n");
		return BUDDY_ERROR_CODE_GENERAL;
	}
}

/**
 * @brief blocking generic HID read routine used by the ADC and counter modes to read
 *  data from the hardware device.  
 * 
 * @param handle hidapi handle pointer
 * @param packet pointer to general_packet_t structure with DAC values to be sent
 * @param streaming boolean indicating if stream mode is MODE_CTRL_STREAM or MODE_CTRL_IMMEDIATE
 * @return int BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
 */
int buddy_read_generic(hid_device *handle, general_packet_t *packet, bool streaming)
{
	return buddy_read_generic_noblock(handle, packet, streaming, BUDDY_WAIT_LONGEST);
}

/**
 * @brief non-blocking generic HID read routine used by the ADC and counter modes to read
 *  data from the hardware device. 
 * 
 * @param handle hidapi handle pointer
 * @param packet pointer to general_packet_t structure with DAC values to be sent
 * @param streaming boolean indicating if stream mode is MODE_CTRL_STREAM or MODE_CTRL_IMMEDIATE
 * @param timeout integer msec for the read operation to try before giving up
 * @return int BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
 */
int buddy_read_generic_noblock(hid_device *handle, general_packet_t *packet, bool streaming, int timeout)
{
	static int decode_status = CODEC_STATUS_FULL;
	static uint8_t in_buf[MAX_IN_SIZE] = { 0 };
	int err_code;

	// 
	// 1. if streaming off/immediate on then read a HID IN packet and decode
	// 2. if streaming on and decode_status equal to CODEC_STATUS_FULL then
	//		read a HID IN packet and decode
	// 3. if streaming on and decode_status equal to CODEC_STATUS_CONTINUE then
	//		decode next packet in the frame

	err_code = BUDDY_ERROR_CODE_OK;
	if ((!streaming) || (decode_status == CODEC_STATUS_FULL)) {
		int res;
		clock_t start_time;

		res = 0;
		start_time = clock();
		while (res == 0) {
			if (((clock() - start_time) / (CLOCKS_PER_SEC / 1000)) >= timeout) {
				return BUDDY_ERROR_CODE_TIMEOUT;
			}

			res = buddy_read_packet(handle, in_buf, MAX_IN_SIZE);

			if (res < 0) {
				critical(("buddy_read_generic_noblock: could not buddy_read_packet\n"));
				return BUDDY_ERROR_CODE_GENERAL;
			}
		}
		
		if (in_buf[BUDDY_APP_CODE_OFFSET] & BUDDY_RESPONSE_TYPE_DATA) {
			// remote data packet
            codec_set_offset_count(0);
            decode_status = codec_decode(in_buf, packet);
		} else if (in_buf[BUDDY_APP_CODE_OFFSET] & BUDDY_RESPONSE_TYPE_STATUS) {
			// remote error status packet -- extract error code
			err_code = in_buf[BUDDY_APP_INDIC_OFFSET];
		} else {
			err_code = BUDDY_ERROR_CODE_INVALID;
		}
	} else if ((streaming) && (decode_status == CODEC_STATUS_CONTINUE)) {
        decode_status = codec_decode(in_buf, packet);
	} else {
		err_code = BUDDY_ERROR_CODE_GENERAL;
	}

	return err_code;
}

/** @brief does some boundary checking on the PWM duty cycle or frequency values
 *   requested.  Translates the requested duty cycle or frequency to the equivalent
 *   PWM reload high and low count values to be sent to the device.  Encodes the packet 
 *   using codec and sends either immediately or if using streaming then waits for codec 
 *   buffer to be full before sending.
*   @param hidapi handle pointer
*   @param pointer to general_packet_t structure with DAC values to be sent
*	@param boolean indicating if stream mode is MODE_CTRL_STREAM or MODE_CTRL_IMMEDIATE
*   @return BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
*/
int buddy_send_pwm(hid_device *handle, general_packet_t *packet, bool streaming)
{
	int i;

	// boundary check PWM value
	// (1) duty cycle - if resolution is RESOLUTION_CTRL_LOW (8-bit) then
	// validate that value is greater than or equal to 1 and less than or
	// equal to 255.  If resolution is RESOLUTION_CTRL_HIGH (16-bit) then
	// validate that value is greater than or equal to 1 and less than or
	// equal to 25 .
	// (2) frequency output - check if (PWM_timbase / (2 * frequency)) as
	// computed as an integer is less than 1 or greater than 255 and if
	// so return an BUDDY_ERROR_OUT_OF_BOUND.  This check is performed
	// regardless of the resolution value

	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
        if (codec_is_channel_active(i)) {
			//printf("buddy_send_pwm(): packet->channels[%d] = %d\n", i, packet->channels[i]);

			switch (codec_get_resolution()) {
				case RESOLUTION_CTRL_SUPER:
					// super (32-bit) transfer disallowed for duty cycle mode
					if (driver_ctx.runtime.pwm_mode == RUNTIME_PWM_MODE_DUTY_CYCLE) {
						return BUDDY_ERROR_CODE_INVALID;
					}

					if ((packet->channels[i] < BUDDY_SUPER_RESOLUTION_MIN) || 
						(packet->channels[i] > BUDDY_SUPER_RESOLUTION_MAX)) {
						return BUDDY_ERROR_CODE_OUT_OF_BOUND;
					}
					break;

				case RESOLUTION_CTRL_HIGH:
					if ((packet->channels[i] < BUDDY_HIGH_RESOLUTION_MIN) || 
						(packet->channels[i] > BUDDY_HIGH_RESOLUTION_MAX)) {
						return BUDDY_ERROR_CODE_OUT_OF_BOUND;
					}
					break;

				case RESOLUTION_CTRL_LOW:
					if ((packet->channels[i] < BUDDY_LOW_RESOLUTION_MIN) || 
						(packet->channels[i] > BUDDY_LOW_RESOLUTION_MAX)) {
						return BUDDY_ERROR_CODE_OUT_OF_BOUND;
					}
					break;

				default:
					// resolution is of unknown value -- fail gracefully
					return BUDDY_ERROR_CODE_INVALID;
			}

			if (driver_ctx.runtime.pwm_mode == RUNTIME_PWM_MODE_FREQUENCY) {
				float check_value;

				switch (driver_ctx.runtime.pwm_timebase) {
					case RUNTIME_PWM_TIMEBASE_SYSCLK:
						check_value	= (BUDDY_SYSCLK / (2 * packet->channels[i]));
						break;

					case RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_4:
						check_value = ((BUDDY_SYSCLK / 4.0) / (2 * packet->channels[i]));
						break;

					case RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_12:
						check_value = ((BUDDY_SYSCLK / 12.0) / (2 * packet->channels[i]));
						break;

					case RUNTIME_PWM_TIMEBASE_TIMER0_OVERFLOW:
					default:
						return BUDDY_ERROR_CODE_INVALID;
				}

				// frequency output sets PCA0CPHN so the resulting check_value
				// must be an unsigned 8-bit integer.
				if ((check_value < BUDDY_LOW_RESOLUTION_MIN) ||
					(check_value > BUDDY_LOW_RESOLUTION_MAX)) {
					return BUDDY_ERROR_CODE_OUT_OF_BOUND;	
				}
			} else if (driver_ctx.runtime.pwm_mode != RUNTIME_PWM_MODE_DUTY_CYCLE) {
				return BUDDY_ERROR_CODE_INVALID;
			}
		}
	}

	return buddy_send_generic(handle, packet, streaming, APP_CODE_PWM);
}

/** @brief encodes the packet using codec and sends either immediately or if using
*			streaming then waits for codec buffer to be full before sending.
*   @param hidapi handle pointer
*   @param pointer to general_packet_t structure with DAC values to be sent
*	@param boolean indicating if stream mode is MODE_CTRL_STREAM or MODE_CTRL_IMMEDIATE
*   @return BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
*/
int buddy_send_dac(hid_device *handle, general_packet_t *packet, bool streaming)
{
	return buddy_send_generic(handle, packet, streaming, APP_CODE_DAC);
}

/**
 * @brief blocking read counter values from the hardware device.
 * 
 * @param hidapi handle pointer
 * @param pointer to general_packet_t structure where counter values to be received 
 * @param streaming boolean indicating if stream mode is MODE_CTRL_STREAM or MODE_CTRL_IMMEDIATE
 * @return int BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
 */
int buddy_read_counter(hid_device *handle, general_packet_t *packet, bool streaming)
{
	return buddy_read_generic(handle, packet, streaming);
}

/** @brief if streaming mode is on then a packet is decoded from the current frame, if
*			the frame buffer is empty then a new HID IN packet is received and decoded.
*   @param hidapi handle pointer
*   @param pointer to general_packet_t structure with ADC values to be received
*	@param boolean indicating if stream mode is MODE_CTRL_STREAM or MODE_CTRL_IMMEDIATE
*   @return BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
*/
int buddy_read_adc(hid_device *handle, general_packet_t *packet, bool streaming)
{
	return buddy_read_generic(handle, packet, streaming);
}

/** @brief empty the system USB HID IN report buffer.  This action must be performed
 *         before immediate reads as the buffer is likely stuffed with the continuous
 *		   stream of previous acquisition
 *  @todo (consider moving this logic internal not invoked by user).
 *  @param handle hidapi internal handle returned from buddy_init
 *  @return BUDDY_ERROR_CODE_OK on success or BUDDY_ERROR_CODE_GENERAL for error.
 */
int buddy_clear(hid_device *handle)
{
	int res;
	uint8_t buffer[MAX_IN_SIZE] = { 0 };
	size_t read_length;

	read_length = (MAX_IN_SIZE - 1);
	while (1) {
		res = hid_read(handle, buffer, read_length);

		if (res > 0) {
			// get next buffer repot
			continue;
		}

		if (res == -1) {
			// error
			return BUDDY_ERROR_CODE_GENERAL;
		}

		if (res == 0) {
			// no more buffer reports, stop.
			return BUDDY_ERROR_CODE_OK;
		}
	}
}

/** @brief writes the bytes that remain in the codec buffer.  This needs to be performed
*		    on the last write to prevent stagnant data remaining in the codec buffer.
*   @param BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
*/
int buddy_flush(hid_device *handle)
{
	if (!handle) {
		return BUDDY_ERROR_CODE_MEMORY;
	}

	//if (codec_byte_offset) {
	if (codec_get_offset_count()) {
    	if (driver_ctx.general.function == GENERAL_CTRL_DAC_ENABLE) {
			out_hold_buf[BUDDY_APP_CODE_OFFSET] = APP_CODE_DAC;
		} else if (driver_ctx.general.function == GENERAL_CTRL_PWM_ENABLE) {
			out_hold_buf[BUDDY_APP_CODE_OFFSET] = APP_CODE_PWM;
		} else {
			return BUDDY_ERROR_CODE_GENERAL;
		}

		out_hold_buf[BUDDY_TYPE_OFFSET] = BUDDY_OUT_DATA_ID;
        out_hold_buf[BUDDY_APP_INDIC_OFFSET] = codec_get_encode_count();

		if (buddy_write_packet(handle, &out_hold_buf[0], MAX_OUT_SIZE) == BUDDY_ERROR_CODE_GENERAL) {
			critical(("buddy_flush: buddy_write_packet call failed\n"));
			return BUDDY_ERROR_CODE_GENERAL;
		}

        codec_set_encode_count(0);
        codec_set_offset_count(0);
	}

	return BUDDY_ERROR_CODE_OK;
}

/**
 * @brief sends a USB OUT request with the configuration data.
 * 
 * @param handle hidapi handle pointer
 * @param control enum of CTRL_CONTROL specifying general, timing, or runtime config
 * @param buffer binary config structure to write to device
 * @param len number of bytes of config structure data to write to device
 * @return int BUDDY_ERROR_CODE_OK for success otherwise error
 */
int buddy_write_config(hid_device *handle, int control, uint8_t *buffer, int len) {
	int rc;

	if ((rc = buddy_write_raw(handle, APP_CODE_CTRL, control, buffer, len)) != BUDDY_ERROR_CODE_OK) {
		critical(("error could not write raw HID packet out\n"));
		return rc;
	}
	
	//err_code = buddy_get_response(handle, NULL, 0);
	short_sleep(100);
	return BUDDY_ERROR_CODE_OK;
} 

/** @brief configure the Buddy device
 *	@param general pointer to ctrl_general_t structure describing the
 *			operation (ADC/DAC), channels, resolution, etc.
 *	@param runtime pointer to ctrl_runtime_t structure describing the
 *			register settings for the ADC and DAC device.
 *  @param timing pointer to ctrl_timing_t structure desribing the
 *			sample period and averaging.
 *  @return BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
 */
// EXPORT
int buddy_configure(hid_device *handle, ctrl_general_t *general, ctrl_runtime_t *runtime, ctrl_timing_t *timing)
{
	int i;
	int rc;

	if ((!handle) || (!general) || (!runtime) || (!timing)) {
		return BUDDY_ERROR_CODE_MEMORY;
	}

	memcpy( (ctrl_general_t *) &driver_ctx.general, 
			general, sizeof(ctrl_general_t));
	memcpy( (ctrl_runtime_t *) &driver_ctx.runtime,
			runtime, sizeof(ctrl_runtime_t));
	memcpy( (ctrl_timing_t *) &driver_ctx.timing,
			timing, sizeof(ctrl_timing_t));

    codec_init(general->channel_mask, general->resolution);

	// if ADC, DAC, or PWM Mode then scale the input sample rate by the number of channels 
	// so that all channels are sampling at the requested rate.  If counter mode then set
	// period to user requested value.

	if (general->function == GENERAL_CTRL_COUNTER_ENABLE) {
		timing->period = swap_uint32(timing->period);
	} else {
        timing->period = swap_uint32(timing->period / codec_get_channel_count());
	}

	if ((rc = buddy_write_config(handle, CTRL_RUNTIME, (uint8_t *) runtime, sizeof(ctrl_runtime_t))) != BUDDY_ERROR_CODE_OK) {
		critical(("error could not write CTRL_RUNTIME configuration\n"));
		return rc;
	}

	if ((rc = buddy_write_config(handle, CTRL_TIMING, (uint8_t *) timing, sizeof(ctrl_timing_t))) != BUDDY_ERROR_CODE_OK) {
		critical(("error could not write CTRL_TIMING configuration\n"));
		return rc;
	}

	if ((rc = buddy_write_config(handle, CTRL_GENERAL, (uint8_t *) general, sizeof(ctrl_general_t))) != BUDDY_ERROR_CODE_OK) {
		critical(("error could not write CTRL_GENERAL configuration\n"));
		return rc;
	}

	return BUDDY_ERROR_CODE_OK; 
}

/**
 * @brief get a response packet from the hardware device.
 * 
 * @param handle hidapi internal handle returned from buddy_init
 * @param res_type integer of enum BUDDY_ERROR_CODE describing success or failure
 * @param buffer pointer location where the frame will be read into
 * @param length size of the location pointer to by the `buffer` parameter
 * @return int BUDDY_RESPONSE_DRV_TYPE_INTERNAL on error, BUDDY_RESPONSE_DRV_TYPE_DATA for
 *  data and BUDDY_RESPONSE_DRV_TYPE_STATUS for status messages
 */
int8_t buddy_get_response(hid_device *handle, uint8_t *buffer, uint8_t length)
{
	uint8_t in_buf[MAX_IN_SIZE] = { 0 };
	uint16_t copy_length;
	int res;
    int j;
	int response_count = 0;

	if ((!buffer) || (length <= 0)) {
		return BUDDY_ERROR_CODE_GENERAL;
	}

	res = 0;
	while (res == 0) {
		res = buddy_read_packet(handle, in_buf, MAX_IN_SIZE - 1);

		if (res == 0) {
			if (response_count >= BUDDY_MAX_IO_ATTEMPTS) {
                debug(("buddy_get_response(): BUDDY_MAX_IO_ATTEMPTS reached\n"));

                printf("buddy_get_response(): timeout in config item request.\n");
                *res_type = BUDDY_ERROR_CODE_TIMEOUT;
                return BUDDY_RESPONSE_DRV_TYPE_INTERNAL;
			}

			response_count++;
			short_sleep(100);
			continue;
		}

		if (res < 0) {
			//critical(("buddy_get_response: could not buddy_read_packet\n"));
			debug(("buddy_get_response(): could not buddy_read_packet\n"));
            *res_type = BUDDY_ERROR_CODE_PROTOCOL;
			return BUDDY_RESPONSE_DRV_TYPE_INTERNAL;
		} else if (res >= 0) {
			if (in_buf[BUDDY_APP_CODE_OFFSET] & BUDDY_RESPONSE_TYPE_DATA) {
				copy_length = (length > (MAX_IN_SIZE - BUDDY_APP_INDIC_OFFSET)) ? (MAX_IN_SIZE - BUDDY_APP_INDIC_OFFSET) : length;
				memcpy(buffer, &in_buf[BUDDY_APP_INDIC_OFFSET], copy_length);
				err_code = BUDDY_ERROR_CODE_OK;
			} else if (in_buf[BUDDY_APP_CODE_OFFSET] & BUDDY_RESPONSE_TYPE_STATUS) {
				err_code = in_buf[BUDDY_APP_INDIC_OFFSET];
			}

			break;
		}
	}

    debug(("buddy_get_response() exit\n"));
    *res_type = BUDDY_ERROR_CODE_OK;
	return BUDDY_RESPONSE_DRV_TYPE_INTERNAL;
}

/**
 * @brief reset the hardware device.
 * 
 * @param handle hidapi internal handle returned from buddy_init
 * @return int BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
 */
int buddy_reset_device(hid_device *handle)
{
	return buddy_write_raw(handle, APP_CODE_RESET, 0x00, (uint8_t *) NULL, 0);
}

/** @brief configure the Buddy device
 *  @param handle hidapi internal handle returned from buddy_init
 *  @param fw_info pointer firmware_info_t structure that will be filled with
 *			firmware device info
 *  @return BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
 */
int buddy_get_firmware_info(hid_device *handle, firmware_info_t *fw_info)
{
	uint8_t in_buf[MAX_IN_SIZE] = { 0 };
	int i;
	int err_code;
	
	if ((!handle) || (!fw_info)) {
		return BUDDY_ERROR_CODE_MEMORY;
	}

	// shraken 7/30/17: cheap hack -- the firmware info get request
	// seems to fail on the buddy_read_packet/hid_read as no IN
	// request gets picked up.  The host does get the packet but
	// might be a bug in hidapi or underlying Win32 impl.  Need
	// to investigate further but right now we just make BUDDY_MAX_IO_ATTEMPTS
	// attempts if the hid_read fails.
	for (i = 0; i < BUDDY_MAX_IO_ATTEMPTS; i++) {
		// request firmware info block
		if (buddy_write_raw(handle, APP_CODE_INFO, 0x00, (uint8_t *)NULL, 0) == BUDDY_ERROR_CODE_OK) {
			short_sleep(100);

			if (buddy_get_response(handle, (uint8_t *) fw_info, sizeof(firmware_info_t)) == BUDDY_ERROR_CODE_OK) {
				// adjust for endian conversion
				fw_info->flash_datetime = swap_uint32(fw_info->flash_datetime);
				fw_info->serial = swap_uint32(fw_info->serial);
				break;
			} else {
				critical(("error could not get response from APP_CODE_INFO request\n"));
				err_code = BUDDY_ERROR_CODE_GENERAL;
			}
		} else {
			critical(("error could not write APP_CODE_INFO over HID\n"));
			err_code = BUDDY_ERROR_CODE_GENERAL;
		}
	}

    printf("buddy_get_firmware_info() exit\n");
	return BUDDY_ERROR_CODE_INVALID;
}

/** @brief initialize the USB HID connection and get the remote firmware device
 *			info and capabilities.
 *  @param hid_info pointer to buddy_hid_info_t structure that will be filled
 *			with USB HID info (mfr name, serial #, etc.)
 *	@param fw_info pointer firmware_info_t structure that will be filled with
 *			firmware device info
 *  @return BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
 */
hid_device* buddy_init(buddy_hid_info_t *hid_info, firmware_info_t *fw_info)
{
	hid_device* handle;

	handle = hidapi_init(hid_info);
	if (!handle) {
		printf("Could not open USB HID connection the Buddy device\n");
		return NULL;
	}

	buddy_get_firmware_info(handle, fw_info);
	return handle;
}

/** @brief cleanup routine for closing hidapi file handle
 *  @param hidapi handle pointer
 *	@param hid_info pointer to structure to store USB device information
 *  @return BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
 */
int buddy_cleanup(hid_device *handle, buddy_hid_info_t *hid_info, bool device_disable)
{
	if ((!handle) || (!hid_info)) {
		return BUDDY_ERROR_CODE_MEMORY;
	}

	ctrl_general_t general_settings = { 0 };

    debug(("buddy_cleanup entered\n"));

	//debugf("buddy_cleanup entered\n");

#if !defined(LABVIEW_BUILD)
	// free the strings from the USB HID device info structure
	if (hid_info->str_serial) {
		free(hid_info->str_serial);
	}

	if (hid_info->str_product) {
		free(hid_info->str_product);
	}

	if (hid_info->str_mfr) {
		free(hid_info->str_mfr);
	}

	if (hid_info->str_index_1) {
		free(hid_info->str_index_1);
	}

	hid_info->str_serial = NULL;
	hid_info->str_product = NULL;
	hid_info->str_mfr = NULL;
	hid_info->str_index_1 = NULL;
#endif

	if (device_disable) {
		general_settings.function = GENERAL_CTRL_NONE;
		if (buddy_write_raw(handle, APP_CODE_CTRL, CTRL_GENERAL, 
			(uint8_t *) &general_settings, sizeof(ctrl_general_t)) != BUDDY_ERROR_CODE_OK) {
			return BUDDY_ERROR_CODE_GENERAL;
		}	
	}

	hid_close(handle);
	hid_exit();

    debug(("buddy_cleanup exit\n"));

	return BUDDY_ERROR_CODE_OK;
}