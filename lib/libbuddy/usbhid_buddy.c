#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <usbhid_buddy.h>
#include <utility.h>
#include <support.h>
#include <time.h>
#include <buddy.h>
#include <utility.h>

static bool _stream_mode = false;
static uint8_t _daq_function = 0;
static uint8_t _chan_mask = 0;
static uint8_t _chan_number = 0;
static uint8_t _chan_enable[BUDDY_CHAN_LENGTH];
static uint8_t _resolution_mode = 0;
static uint8_t _data_size = 2;

static uint8_t codec_byte_offset = 0;
static uint8_t encode_count = 0;
static uint8_t decode_count = 0;

static uint8_t out_hold_buf[MAX_OUT_SIZE] = { 0 };
static buddy_driver_context driver_ctx = { 0 };

char *fw_info_dac_type_names[FIRMWARE_INFO_DAC_TYPE_LENGTH] = {
	"None",
	"TI TLV5630 (12-bit)",
	"TI TLV5631 (10-bit)",
	"TI TLV5632 (8-bit)",
};

/** @brief prints a dump of the fw_info_dac_type_names ASCIIz
 *			strings to the console.
 *  @return Void.
*/
void print_fw_info_dac_types(void)
{
	int i;

	printf("print_fw_info_dac_types()\n");
	for (i = 0; i < FIRMWARE_INFO_DAC_TYPE_LENGTH; i++) {
		printf("%s\n", fw_info_dac_type_names[i]);
	}
}

/** @brief prints a hex dump of the internal HID IN packet buffer
 *  @param buffer pointer to IN USBHID packet
 *  @param length number of the bytes specified by the pointer buffer
 *  @return Void.
*/
void print_buffer(uint8_t *buffer, uint8_t length)
{
	int i;

	for (i = 0; i < length; i++) {
		printf("%02X:", *(buffer + i));
	}
	printf("\n\n");
}

/** @brief prints a dump of ADC values for incoming HID IN packet buffer
*   @param buffer pointer to IN USBHID packet
*   @return Void.
*/
void print_buffer_simple(uint16_t *buffer)
{
	int i;
	uint8_t offset;
	uint16_t value;

	offset = 0;
	for (i = 0; i < 8; i++) {
		value = (uint16_t)* (buffer + offset);
		value = swap_uint16(value);

		printf("adc_result[%d] = %u (0x%x)\r\n", i, value, value);
		offset += 1;
	}
	printf("\n\n");
}

int number_channels(uint8_t channel_mask) 
{
	int i;
	int channel_count = 0;

	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (channel_mask & (1 << i)) {
			channel_count++;
		}
	}

	return channel_count;
}

int encode(uint8_t *frame, general_packet_t *packet)
{
	uint8_t i;

	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (_chan_enable[i]) {
			if (_resolution_mode == RESOLUTION_CTRL_SUPER) {
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset) = ((packet->channels[i] & 0xFF000000) >> 24);
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 1) = ((packet->channels[i] & 0xFF0000) >> 16);
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 2) = ((packet->channels[i] & 0xFF00) >> 8);
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 3) = (packet->channels[i] & 0xFF);
			} else if (_resolution_mode == RESOLUTION_CTRL_HIGH) {
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset) = ((packet->channels[i] & 0xFF00) >> 8);
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 1) = (packet->channels[i] & 0xFF);
			} else if (_resolution_mode == RESOLUTION_CTRL_LOW) {
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset) = (packet->channels[i] & 0xFF);
			} else {
				// resolution not defined, return error
				return CODEC_STATUS_ERROR;
			}
			
			codec_byte_offset += _data_size;
		}
	}

	// check if subsequent packet will overflow buffer
	if ((codec_byte_offset + (_data_size * _chan_number)) > (MAX_REPORT_SIZE - 3)) {
		codec_byte_offset = 0;
		return CODEC_STATUS_FULL;
	} else {
		return CODEC_STATUS_CONTINUE;
	}
}

int decode(uint8_t *frame, general_packet_t *packet)
{
	uint8_t count;
	int i;

	count = *(frame + BUDDY_APP_INDIC_OFFSET);
	//printf("decode() with count = %d\n", count);

	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (_chan_enable[i]) {
			if (_resolution_mode == RESOLUTION_CTRL_SUPER) {
				packet->channels[i] = (*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset) << 24);
				packet->channels[i] |= (*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 1) << 16);
				packet->channels[i] |= (*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 2) << 8);
				packet->channels[i] |= *(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 3);
			} else if (_resolution_mode == RESOLUTION_CTRL_HIGH) {
				if ((*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset)) & 0x80) {
					packet->channels[i] = ((*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset)) << 8);
					packet->channels[i] |= *(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 1);
					
					// sign extension
					packet->channels[i] = packet->channels[i] | 0xFFFF0000;
				} else {
					packet->channels[i] = (*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset) << 8);
					packet->channels[i] |= *(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 1);
				}
			} else if (_resolution_mode == RESOLUTION_CTRL_LOW) {
				packet->channels[i] = (*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset));
			} else {
				// resolution not defined, return error
				return CODEC_STATUS_ERROR;
			}

			codec_byte_offset += _data_size;
		}
	}

	decode_count++;

	// check if the next packet can fit in the packed array
	if (((codec_byte_offset + (_data_size * _chan_number)) > (MAX_REPORT_SIZE - 3)) ||
	     (decode_count >= count)) {
		codec_byte_offset = 0;
		decode_count = 0;

		return CODEC_STATUS_FULL;
	} else {
		return CODEC_STATUS_CONTINUE;
	}
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

	//debugf("hidapi_init entered\n");

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
	
	if (buddy_write_packet(handle, &out_buf[0], MAX_OUT_SIZE) == -1) {
		return BUDDY_ERROR_CODE_GENERAL;
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
	static int count = 0;
	int res;

	res = hid_write(handle, buffer, length);
	if (res < 0) {
		critical(("buddy_write_packet: hid_write call failed, error = %ls handle = %p\n", 
			hid_error(handle), handle));
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

	err_code = encode(out_hold_buf, packet);
	encode_count++;

	if ((!streaming) || (err_code == CODEC_STATUS_FULL)) {
		out_hold_buf[BUDDY_TYPE_OFFSET] = BUDDY_OUT_DATA_ID;
		out_hold_buf[BUDDY_APP_CODE_OFFSET] = type;
		out_hold_buf[BUDDY_APP_INDIC_OFFSET] = encode_count;

		//printf("buddy_send_pwm() with encode_count = %d\n", encode_count);
		if (buddy_write_packet(handle, &out_hold_buf[0], MAX_OUT_SIZE) == -1) {
			critical(("buddy_send_pwm: buddy_write_packet call failed\n"));
			return BUDDY_ERROR_CODE_GENERAL;
		}

		encode_count = 0;
		codec_byte_offset = 0;

		return BUDDY_ERROR_CODE_OK;
	} else if (err_code == CODEC_STATUS_CONTINUE) {
		return BUDDY_ERROR_CODE_OK;
	} else {
		printf("buddy_send_pwm: err_code = BUDDY_ERROR_CODE_GENERAL\n");
		return BUDDY_ERROR_CODE_GENERAL;
	}
}

int buddy_read_generic(hid_device *handle, general_packet_t *packet, bool streaming)
{
	return buddy_read_generic_noblock(handle, packet, streaming, BUDDY_WAIT_LONGEST);
}

int buddy_read_generic_noblock(hid_device *handle, general_packet_t *packet, bool streaming, int timeout)
{
	static int decode_status = CODEC_STATUS_FULL;
	static uint8_t in_buf[MAX_IN_SIZE] = { 0 };
	int err_code;
	int res;
	int i;

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
			codec_byte_offset = 0;
			decode_status = decode(in_buf, packet);
		} else if (in_buf[BUDDY_APP_CODE_OFFSET] & BUDDY_RESPONSE_TYPE_STATUS) {
			// remote error status packet -- extract error code
			err_code = in_buf[BUDDY_APP_INDIC_OFFSET];
		} else {
			err_code = BUDDY_ERROR_CODE_INVALID;
		}
	} else if ((streaming) && (decode_status == CODEC_STATUS_CONTINUE)) {
		decode_status = decode(in_buf, packet);
	} else {
		err_code = BUDDY_ERROR_CODE_GENERAL;
	}

	return err_code;
}

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
		if (_chan_enable[i]) {
			//printf("buddy_send_pwm(): packet->channels[%d] = %d\n", i, packet->channels[i]);

			switch (driver_ctx.general.resolution) {
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
						check_value	= (SYSCLK / (2 * packet->channels[i]));
						break;

					case RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_4:
						check_value = ((SYSCLK / 4.0) / (2 * packet->channels[i]));
						break;

					case RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_12:
						check_value = ((SYSCLK / 12.0) / (2 * packet->channels[i]));
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

int buddy_read_adc_noblock(hid_device *handle, general_packet_t *packet, bool streaming, int timeout)
{
	return buddy_read_generic_noblock(handle, packet, streaming, timeout);
}

/** @brief empty the system USB HID IN report buffer.  This action must be performed
 *         before immediate reads as the buffer is likely stuffed with the continuous
 *		   stream of previous acquisition
 *  @todo (consider moving this logic internal not invoked by user).
 *  @param handle hidapi internal handle returned from buddy_init
 *  @return -1 on failure, 0 on success. 
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
			return -1;
		}

		if (res == 0) {
			// no more buffer reports, stop.
			return 0;
		}
	}
}

/** @brief writes the bytes that remain in the codec buffer.  This needs to be performed
*		    on the last write to prevent stagnant data remaining in the codec buffer.
*   @param BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
*/
int buddy_flush(hid_device *handle)
{
	if (codec_byte_offset) {
		if (_daq_function == GENERAL_CTRL_DAC_ENABLE) {
			out_hold_buf[BUDDY_APP_CODE_OFFSET] = APP_CODE_DAC;
		} else if (_daq_function == GENERAL_CTRL_PWM_ENABLE) {
			out_hold_buf[BUDDY_APP_CODE_OFFSET] = APP_CODE_PWM;
		} else {
			return BUDDY_ERROR_CODE_GENERAL;
		}

		out_hold_buf[BUDDY_TYPE_OFFSET] = BUDDY_OUT_DATA_ID;
		out_hold_buf[BUDDY_APP_INDIC_OFFSET] = encode_count;

		if (buddy_write_packet(handle, &out_hold_buf[0], MAX_OUT_SIZE) == -1) {
			critical(("buddy_flush: buddy_write_packet call failed\n"));
			return BUDDY_ERROR_CODE_GENERAL;
		}

		codec_byte_offset = 0;
		encode_count = 0;
	}

	return BUDDY_ERROR_CODE_OK;
}

/** @brief returns the number of active channels by looking at the channel_mask
*			and counting them.
*   @param number of channels activated in the current request
*/
int buddy_count_channels(uint8_t chan_mask)
{
	int i;
	int adc_channel_count = 0;

	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (chan_mask & (1 << i)) {
			adc_channel_count++;
		}
	}

	return adc_channel_count;
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
	char buffer[128];
	int i;
	int j;
	int8_t err_code;

	buddy_cfg_reg_t cfg_regs[NUMBER_CFG_REG_ENTRIES] = {
		{
		  	CTRL_RUNTIME, (uint8_t *) runtime, sizeof(ctrl_runtime_t),
		},
		{
			CTRL_TIMING, (uint8_t *) timing, sizeof(ctrl_timing_t),
		},
		{
			CTRL_GENERAL, (uint8_t *) general, sizeof(ctrl_general_t),
		}
	};

	if ((!handle) || (!general) || (!runtime) || (!timing)) {
		return BUDDY_ERROR_CODE_MEMORY;
	}

	memcpy( (ctrl_general_t *) &driver_ctx.general, 
			general, sizeof(ctrl_general_t));
	memcpy( (ctrl_runtime_t *) &driver_ctx.runtime,
			runtime, sizeof(ctrl_runtime_t));
	memcpy( (ctrl_timing_t *) &driver_ctx.timing,
			timing, sizeof(ctrl_timing_t));

	_daq_function = general->function;
	_chan_mask = general->channel_mask;
	_resolution_mode = general->resolution;

	switch (_resolution_mode) {
		case RESOLUTION_CTRL_SUPER:
			_data_size = BUDDY_DATA_SIZE_SUPER;
			break;

		case RESOLUTION_CTRL_HIGH:
			_data_size = BUDDY_DATA_SIZE_HIGH;
			break;

		case RESOLUTION_CTRL_LOW:
			_data_size = BUDDY_DATA_SIZE_LOW;
			break;

		default:
			return 10;
	}

	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (_chan_mask & (1 << i)) {
			_chan_enable[i] = 1;
			_chan_number++;
		} else {
			_chan_enable[i] = 0;
		}
	}

	// if ADC, DAC, or PWM Mode then scale the input sample rate by the number of channels 
	// so that all channels are sampling at the requested rate.  If counter mode then set
	// period to user requested value.

	if (general->function == GENERAL_CTRL_COUNTER_ENABLE) {
		timing->period = swap_uint32(timing->period);
	} else {
		timing->period = swap_uint32(timing->period / buddy_count_channels(general->channel_mask));
	}

	for (i = 0; i < NUMBER_CFG_REG_ENTRIES; i++) {
		if (buddy_write_raw(handle, APP_CODE_CTRL, 
							cfg_regs[i].type_indic, 
							cfg_regs[i].record_cfg, 
							cfg_regs[i].record_len) == BUDDY_ERROR_CODE_OK) {
							
			if ((err_code = buddy_get_response(handle, NULL, 0)) == BUDDY_ERROR_CODE_OK) {
				short_sleep(100);
			} else {
				short_sleep(100);
			}
		}
	}

	return BUDDY_ERROR_CODE_OK; 
}

int8_t buddy_get_response(hid_device *handle, uint8_t *buffer, uint8_t length)
{
	uint8_t in_buf[MAX_IN_SIZE] = { 0 };
	uint16_t copy_length;
	int res;
	int err_code = BUDDY_ERROR_CODE_OK;
	int response_count = 0;

	res = 0;
	while (res == 0) {
		res = buddy_read_packet(handle, in_buf, MAX_IN_SIZE - 1);

		if (res == 0) {
			if (response_count >= BUDDY_MAX_IO_ATTEMPTS) {
				err_code = BUDDY_ERROR_CODE_TIMEOUT;
				break;
			}

			response_count++;
			short_sleep(1);
			continue;
		}

		if (res < 0) {
			critical(("buddy_get_response: could not buddy_read_packet\n"));
			err_code = BUDDY_ERROR_CODE_PROTOCOL;
			break;
		} else if (res >= 0) {
			if (in_buf[BUDDY_APP_CODE_OFFSET] & BUDDY_RESPONSE_TYPE_DATA) {
				if ((buffer) && (length > 0) && (res > 0)) {
					copy_length = (length > (MAX_IN_SIZE - BUDDY_APP_INDIC_OFFSET)) ? (MAX_IN_SIZE - BUDDY_APP_INDIC_OFFSET) : length;
					memcpy(buffer, &in_buf[BUDDY_APP_INDIC_OFFSET], copy_length);
					err_code = BUDDY_ERROR_CODE_OK;
				} else {
					err_code = BUDDY_ERROR_CODE_INVALID;
				}
			} else if (in_buf[BUDDY_APP_CODE_OFFSET] & BUDDY_RESPONSE_TYPE_STATUS) {
				err_code = in_buf[BUDDY_APP_INDIC_OFFSET];
			} else {
				err_code = BUDDY_ERROR_CODE_INVALID;
			}

			break;
		}
	}

	return err_code;
}

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
	int res;
	int i;
	int err_code = BUDDY_ERROR_CODE_OK;
	
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
			}
		}
		else {
			critical(("buddy_get_firmware_info(): failed on buddy_write_raw\n"));
			err_code = -1;
		}
	}

	return err_code;
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
	char buffer[128];

	//debugf("buddy_init entered\n");

	handle = hidapi_init(hid_info);
	if (!handle) {
		printf("Could not open USB HID connection the Buddy device\n");
		return NULL;
	}

	/*
	sprintf(buffer, "handle = %p\r\n", handle);
	debugf(buffer);
	*/

	buddy_get_firmware_info(handle, fw_info);
	return handle;
}

/** @brief trigger a start of conversion for either DAC or ADC
*   @param handle pointer
*/
int buddy_trigger(hid_device *handle)
{
	return buddy_write_raw(handle, APP_CODE_TRIGGER, 0x00, (uint8_t *) NULL, 0);
}

/** @brief cleanup routine for closing hidapi file handle
 *  @param hidapi handle pointer
 *	@param hid_info pointer to structure to store USB device information
 *  @return BUDDY_ERROR_CODE_OK on success, BUDDY_ERROR_CODE_GENERAL on failure.
 */
int buddy_cleanup(hid_device *handle, buddy_hid_info_t *hid_info, bool device_disable)
{
	ctrl_general_t general_settings = { 0 };

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

	return BUDDY_ERROR_CODE_OK;
}