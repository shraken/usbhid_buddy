#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <usbhid_buddy.h>
#include <utility.h>
#include <support.h>
#include <buddy.h>
#include <utility.h>

static bool _stream_mode = false;
static uint8_t _chan_mask = 0;
static uint8_t _chan_number = 0;
static uint8_t _chan_enable[BUDDY_CHAN_LENGTH];
static uint8_t _resolution_mode = 0;
static uint8_t _data_size = 2;

static uint8_t codec_byte_offset = 0;
static uint8_t encode_count = 0;
static uint8_t decode_count = 0;

static uint8_t out_hold_buf[MAX_OUT_SIZE] = { 0 };

char *fw_info_dac_type_names[FIRMWARE_INFO_DAC_TYPE_LENGTH] = {
	"None",
	"TI TLV5630 (12-bit)",
	"TI TLV5631 (10-bit)",
	"TI TLV5632 (8-bit)",
};

void print_fw_info_dac_types(void)
{
	int i;

	printf("print_fw_info_dac_types()\n");
	for (i = 0; i < FIRMWARE_INFO_DAC_TYPE_LENGTH; i++) {
		printf("%s\n", fw_info_dac_type_names[i]);
	}
}

void print_buffer(uint8_t *buffer, uint8_t length)
{
	int i;

	for (i = 0; i < length; i++) {
		printf("%02X:", *(buffer + i));
	}
	printf("\n\n");
}

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
			if (_resolution_mode == RESOLUTION_CTRL_HIGH) {
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
			if (_resolution_mode == RESOLUTION_CTRL_HIGH) {
				packet->channels[i] = (*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset) << 8);
				packet->channels[i] |= *(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 1);
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

int buddy_write_raw(hid_device *handle, uint8_t code, uint8_t indic, uint8_t *raw, uint8_t length)
{
	unsigned char out_buf[MAX_OUT_SIZE] = { 0 };
	uint8_t copy_length;

	out_buf[BUDDY_TYPE_OFFSET] = BUDDY_OUT_DATA_ID;
	out_buf[BUDDY_APP_CODE_OFFSET] = code;
	out_buf[BUDDY_APP_INDIC_OFFSET] = indic;

	copy_length = (length > (MAX_OUT_SIZE - 3)) ? (MAX_OUT_SIZE - 3) : length;
	//debug(("buddy_write_raw: copy_length = %d\r\n", copy_length));

	if (raw) {
		memcpy(&out_buf[BUDDY_APP_VALUE_OFFSET], raw, copy_length);
	}
	
	if (buddy_write_packet(handle, &out_buf[0], MAX_OUT_SIZE) == -1) {
		//printf("buddy_write_raw: buddy_write_packet() failed\n");
		return BUDDY_ERROR_GENERAL;
	}

	return BUDDY_ERROR_OK;
}

int buddy_write_packet(hid_device *handle, unsigned char *buffer, int length)
{
	static int count = 0;
	int res;

	//printf("buddy_write_packet() invoked\r\n");

	res = hid_write(handle, buffer, length);
	if (res < 0) {
		/*
		critical(("buddy_write_packet: hid_write call failed, error = %ls handle = %p\n", 
					hid_error(handle), handle));
		*/
		printf("buddy_write_packet: hid_write call failed, error = %ls handle = %p\n", 
					hid_error(handle), handle);
		return BUDDY_ERROR_GENERAL;
	} else {
		//printf("buddy_write_packet successful, count = %d\n", count++);
	}

	return BUDDY_ERROR_OK;
}

int buddy_read_packet(hid_device *handle, unsigned char *buffer, int length)
{
	int res;

	res = hid_read(handle, buffer, length);
	return res;
}

int buddy_send_dac(hid_device *handle, general_packet_t *packet, bool streaming)
{
	int err_code;

	err_code = encode(out_hold_buf, packet);
	encode_count++;

	if ((!streaming) || (err_code == CODEC_STATUS_FULL)) {
		out_hold_buf[BUDDY_TYPE_OFFSET] = BUDDY_OUT_DATA_ID;
		out_hold_buf[BUDDY_APP_CODE_OFFSET] = APP_CODE_DAC;
		out_hold_buf[BUDDY_APP_INDIC_OFFSET] = encode_count;

		printf("buddy_send_dac() with encode_count = %d\n", encode_count);
		if (buddy_write_packet(handle, &out_hold_buf[0], MAX_OUT_SIZE) == -1) {
			//critical(("buddy_send_dac: buddy_write_packet call failed\n"));
			printf("buddy_send_dac: buddy_write_packet call failed\n");
			return BUDDY_ERROR_GENERAL;
		}

		encode_count = 0;
		codec_byte_offset = 0;

		return BUDDY_ERROR_OK;
	} else if (err_code == CODEC_STATUS_CONTINUE) {
		//printf("encode_packet: continue\r\n");
		//debugf("buddy_send_dac exited 2\n");
		return BUDDY_ERROR_OK;
	} else {
		printf("buddy_send_dac: err_code = BUDDY_ERROR_GENERAL\n");
		return BUDDY_ERROR_GENERAL;
	}
}

int buddy_read_adc(hid_device *handle, general_packet_t *packet, bool streaming)
{
	static int encode_status = CODEC_STATUS_FULL;
	static uint8_t in_buf[MAX_IN_SIZE] = { 0 };
	int err_code;
	int res;
	int i;

	// 
	// 1. if streaming off/immediate on then read a HID IN packet and decode
	// 2. if streaming on and encode_status equal to CODEC_STATUS_FULL then
	//		read a HID IN packet and decode
	// 3. if streaming on and encode_status equal to CODEC_STATUS_CONTINUE then
	//		decode next packet in the frame

	err_code = BUDDY_ERROR_OK;
	if ((!streaming) || (encode_status == CODEC_STATUS_FULL)) {
		res = 0;
		while (res == 0) {
			res = buddy_read_packet(handle, in_buf, MAX_IN_SIZE);

			if (res < 0) {
				printf("buddy_read_adc: could not buddy_read_packet\n");
				//critical(("buddy_read_adc: could not buddy_read_packet\n"));
				//debugf("res < 0: failed on buddy_read_packet\n");
				return -1;
			}
		}

		//printf("in_buf[1] = %02x\r\n", in_buf[1]);
		//printf("in_counter (valid) = %d\n", in_buf[1] & 0x7F);
		//print_buffer(in_buf, MAX_IN_SIZE);
		
		if (in_buf[BUDDY_APP_CODE_OFFSET] & BUDDY_RESPONSE_VALID) {
			codec_byte_offset = 0;
			encode_status = decode(in_buf, packet);
		} else {
			// filler packet was detected
			err_code = BUDDY_ERROR_INVALID;
		}
	} else if ((streaming) && (encode_status == CODEC_STATUS_CONTINUE)) {
		encode_status = decode(in_buf, packet);
	} else {
		err_code = BUDDY_ERROR_GENERAL;
	}

	return err_code;
}

int buddy_flush(hid_device *handle)
{
	if (codec_byte_offset) {
		printf("flushing the buffer\r\n");

		out_hold_buf[BUDDY_TYPE_OFFSET] = BUDDY_OUT_DATA_ID;
		out_hold_buf[BUDDY_APP_CODE_OFFSET] = APP_CODE_DAC;
		out_hold_buf[BUDDY_APP_INDIC_OFFSET] = encode_count;

		if (buddy_write_packet(handle, &out_hold_buf[0], MAX_OUT_SIZE) == -1) {
			critical(("buddy_flush: buddy_write_packet call failed\n"));
			return BUDDY_ERROR_GENERAL;
		}

		codec_byte_offset = 0;
		encode_count = 0;
	}

	return BUDDY_ERROR_OK;
}

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

int buddy_configure(hid_device *handle, ctrl_general_t *general, ctrl_runtime_t *runtime, ctrl_timing_t *timing)
{
	char buffer[128];
	int i;

	if ((!handle) || (!general) || (!runtime) || (!timing)) {
		return BUDDY_ERROR_MEMORY;
	}

	/*
	debugf("buddy_configure entered\r\n");
	sprintf(buffer, "handle = %p\r\n", handle);
	debugf(buffer);
	sprintf(buffer, "general = %p\r\n", general);
	debugf(buffer);
	sprintf(buffer, "runtime = %p\r\n", runtime);
	debugf(buffer);
	sprintf(buffer, "timing = %p\r\n", timing);
	debugf(buffer);
	*/

	_chan_mask = general->channel_mask;
	_resolution_mode = general->resolution;

	if (_resolution_mode == RESOLUTION_CTRL_HIGH) {
		_data_size = 2;
	} else if (_resolution_mode == RESOLUTION_CTRL_LOW) {
		_data_size = 1;
	} else {
		return BUDDY_ERROR_GENERAL;
	}

	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (_chan_mask & (1 << i)) {
			_chan_enable[i] = 1;
			_chan_number++;
		} else {
			_chan_enable[i] = 0;
		}
	}

	// scale the input sample rate by the number of channels so that all
	// ADC channels are sampling at the requested rate
	timing->period = swap_uint32(timing->period / buddy_count_channels(general->channel_mask));

	// registers
	if (buddy_write_raw(handle, APP_CODE_CTRL, CTRL_RUNTIME, 
			(uint8_t *) runtime, sizeof(ctrl_runtime_t)) != BUDDY_ERROR_OK) {
		return BUDDY_ERROR_GENERAL;
	}
	short_sleep(100);

	// timing
	if (buddy_write_raw(handle, APP_CODE_CTRL, CTRL_TIMING, 
			(uint8_t *) timing, sizeof(ctrl_timing_t)) != BUDDY_ERROR_OK) {
		return BUDDY_ERROR_GENERAL;
	}
	short_sleep(100);

	// general
	if (buddy_write_raw(handle, APP_CODE_CTRL, CTRL_GENERAL, 
			(uint8_t *) general, sizeof(ctrl_general_t)) != BUDDY_ERROR_OK) {
		return BUDDY_ERROR_GENERAL;
	}
	short_sleep(100);

	return BUDDY_ERROR_OK; 
}

int buddy_get_firmware_info(hid_device *handle, firmware_info_t *fw_info)
{
	uint8_t in_buf[MAX_IN_SIZE] = { 0 };
	int res;
	int i;
	int err_code = BUDDY_ERROR_OK;
	
	// shraken 7/30/17: cheap hack -- the firmware info get request
	// seems to fail on the buddy_read_packet/hid_read as no IN
	// request gets picked up.  The host does get the packet but
	// might be a bug in hidapi or underlying Win32 impl.  Need
	// to investigate further but right now we just make BUDDY_MAX_IO_ATTEMPTS
	// attempts if the hid_read fails.
	for (i = 0; i < BUDDY_MAX_IO_ATTEMPTS; i++) {
		//printf("buddy_get_firmware_info() attempt = %d\n", i);

		// request firmware info block
		if (buddy_write_raw(handle, APP_CODE_INFO, 0x00, (uint8_t *)NULL, 0) == BUDDY_ERROR_OK) {
			short_sleep(100);

			res = 0;
			while (res == 0) {
				res = buddy_read_packet(handle, in_buf, MAX_IN_SIZE - 1);

				if (res < 0) {
					critical(("buddy_read_adc: could not buddy_read_packet\n"));
					return -1;
				}
				else if (res >= 0) {
					break;
				}
			}

			if (res > 0) {
				//printf("buddy_get_firmware_info(): copy firmware info\n");
				memcpy(fw_info, &in_buf[BUDDY_APP_INDIC_OFFSET], sizeof(firmware_info_t));

				// adjust for endian conversion
				fw_info->flash_datetime = swap_uint32(fw_info->flash_datetime);
				fw_info->serial = swap_uint32(fw_info->serial);

				break;
			}
		}
		else {
			printf("buddy_get_firmware_info(): failed on buddy_write_raw\n");
			err_code = -1;
		}
	}

	return err_code;
}

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

int buddy_trigger(hid_device *handle)
{
	return buddy_write_raw(handle, APP_CODE_TRIGGER, 0x00, 
				(uint8_t *) NULL, 0);
}

int buddy_cleanup(hid_device *handle, buddy_hid_info_t *hid_info)
{
	ctrl_general_t general_settings = { 0 };

	//debugf("buddy_cleanup entered\n");

	general_settings.function = GENERAL_CTRL_NONE;

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

	general_settings.function = GENERAL_CTRL_NONE;

	if (buddy_write_raw(handle, APP_CODE_CTRL, CTRL_GENERAL, 
		(uint8_t *) &general_settings, sizeof(ctrl_general_t)) != BUDDY_ERROR_OK) {
		return BUDDY_ERROR_GENERAL;
	}

	hid_close(handle);
	hid_exit();

	return BUDDY_ERROR_OK;
}