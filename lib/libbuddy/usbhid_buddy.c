#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <utility.h>
#include <support.h>
#include <buddy.h>
#include <usbhid_buddy.h>

static bool _stream_mode = false;
static uint8_t _chan_mask = 0;
static uint8_t _res_mask = 0;

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

hid_device* hidapi_init()
{
	hid_device *handle;
	wchar_t wstr[MAX_STR];
	struct hid_device_info *devs, *cur_dev;
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

	/*
	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	if (res < 0) {
		critical(("hidapi_init(): Unable to read manufacturer string\n"));
	}
	debug(("hidapi_init(): Manufacturer String: %ls\n", wstr));

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_STR);
	if (res < 0) {
		critical(("hidapi_init(): Unable to read product string\n"));
	}
	debug(("hidapi_init(): Product String: %ls\n", wstr));

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	if (res < 0) {
		critical(("hidapi_init(): Unable to read serial number string\n"));
	}
	debug(("hidapi_init(): Serial Number String: (%d) %ls\n", wstr[0], wstr));

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	if (res < 0)
		critical(("Unable to read indexed string 1\n"));
	debug(("Indexed String 1: %ls\n", wstr));
	*/

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
	debug(("buddy_write_raw: copy_length = %d\r\n", copy_length));

	if (raw) {
		memcpy(&out_buf[BUDDY_APP_VALUE_OFFSET], raw, copy_length);
	}
	
	if (buddy_write_packet(handle, &out_buf[0], MAX_OUT_SIZE) == -1) {
		return BUDDY_ERROR_GENERAL;
	}

	return BUDDY_ERROR_OK;
}

int buddy_write_packet(hid_device *handle, unsigned char *buffer, int length)
{
	int res;

	res = hid_write(handle, buffer, length);
	if (res < 0) {
		critical(("buddy_write_packet: hid_write call failed, error = %ls handle = %p\n", 
					hid_error(handle), handle));
		return BUDDY_ERROR_GENERAL;
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
	unsigned char out_buf[MAX_OUT_SIZE] = { 0 };
	uint8_t err_code;

	err_code = encode_packet(packet);

	if ((!streaming) || (err_code == CODEC_STATUS_FULL)) {
		//printf("encode_packet: full, sending now\r\n");

		out_buf[BUDDY_TYPE_OFFSET] = BUDDY_OUT_DATA_ID;
		out_buf[BUDDY_APP_CODE_OFFSET] = APP_CODE_DAC;
		memcpy(&out_buf[BUDDY_APP_INDIC_OFFSET], codec_get_buffer(), (MAX_OUT_SIZE - 3));

		if (buddy_write_packet(handle, &out_buf[0], MAX_OUT_SIZE) == -1) {
			critical(("buddy_send_dac: buddy_write_packet call failed\n"));
			return BUDDY_ERROR_GENERAL;
		}

		codec_reset();
		codec_clear();

		return BUDDY_ERROR_OK;
	} else if (err_code == CODEC_STATUS_CONTINUE) {
		//printf("encode_packet: continue\r\n");
		return BUDDY_ERROR_OK;
	} else {
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
			res = buddy_read_packet(handle, in_buf, MAX_IN_SIZE - 1);

			if (res < 0) {
				critical(("buddy_read_adc: could not buddy_read_packet\n"));
				return -1;
			}
		}

		if (in_buf[BUDDY_APP_CODE_OFFSET] & BUDDY_RESPONSE_VALID) {
			printf("in_counter (valid) = %d\n", in_buf[1] & 0x7F);
			print_buffer(in_buf, MAX_IN_SIZE);

			codec_reset();
			codec_clear();

			encode_status = decode_packet( (buddy_frame_t *) (in_buf + BUDDY_APP_INDIC_OFFSET), 
											packet);
		} else {
			err_code = BUDDY_ERROR_INVALID;
		}
	}
	else if ((streaming) && (encode_status == CODEC_STATUS_CONTINUE)) {
		encode_status = decode_packet( (buddy_frame_t *) (in_buf + BUDDY_APP_INDIC_OFFSET), 
									    packet);
	} else {
		err_code = BUDDY_ERROR_GENERAL;
	}

	return err_code;
}

int buddy_flush(hid_device *handle)
{
	unsigned char out_buf[MAX_OUT_SIZE] = { 0 };

	out_buf[BUDDY_TYPE_OFFSET] = BUDDY_OUT_DATA_ID;
	out_buf[BUDDY_APP_CODE_OFFSET] = APP_CODE_DAC;
	memcpy(&out_buf[BUDDY_APP_VALUE_OFFSET], codec_get_buffer(), (MAX_OUT_SIZE - 3));

	if (buddy_write_packet(handle, &out_buf[0], MAX_OUT_SIZE) == -1) {
		critical(("buddy_flush: buddy_write_packet call failed\n"));
		return BUDDY_ERROR_GENERAL;
	}

	codec_reset();
	codec_clear();

	return BUDDY_ERROR_OK;
}

int buddy_count_channels(uint8_t chan_mask)
{
	int i;
	int adc_channel_count = 0;

	for (i = BUDDY_CHAN_7; i <= BUDDY_CHAN_0; i++) {
		if (chan_mask & (1 << i)) {
			adc_channel_count++;
		}
	}

	return adc_channel_count;
}

hid_device* buddy_init(ctrl_general_t *general, ctrl_runtime_t *runtime, ctrl_timing_t *timing)
{
	hid_device* handle;
	unsigned char out_buf[MAX_OUT_SIZE] = { 0 };

	_chan_mask = general->channel_mask;
	_res_mask = general->resolution;

	printf("buddy_init: _chan_mask = %02x\r\n", general->channel_mask);
	printf("buddy_init: _res_mask = %02x\r\n", general->resolution);

	if (!is_codec_initialized()) {
		if (codec_init(general->mode == MODE_CTRL_STREAM, _chan_mask, _res_mask) != CODEC_STATUS_NOERR) {
			printf("buddy_init: codec_init call failed\n");
			return BUDDY_ERROR_GENERAL;
		}
	}

	// scale the input sample rate by the number of channels so that all
	// ADC channels are sampling at the requested rate
	timing->period = swap_uint32(timing->period / buddy_count_channels(general->channel_mask));

	handle = hidapi_init();

	if (!handle) {
		return NULL;
	}

	// registers
	if (buddy_write_raw(handle, APP_CODE_CTRL, CTRL_RUNTIME, 
			(uint8_t *) runtime, sizeof(ctrl_runtime_t)) != BUDDY_ERROR_OK) {
		return NULL;
	}
	short_sleep(100);

	// timing
	if (buddy_write_raw(handle, APP_CODE_CTRL, CTRL_TIMING, 
			(uint8_t *) timing, sizeof(ctrl_timing_t)) != BUDDY_ERROR_OK) {
		return NULL;
	}
	short_sleep(100);

	// general
	if (buddy_write_raw(handle, APP_CODE_CTRL, CTRL_GENERAL, 
			(uint8_t *) general, sizeof(ctrl_general_t)) != BUDDY_ERROR_OK) {
		return NULL;
	}
	short_sleep(100);
	
	return handle;
}

int buddy_trigger(hid_device *handle)
{
	return buddy_write_raw(handle, APP_CODE_TRIGGER, 0x00, 
				(uint8_t *) NULL, 0);
}

int buddy_cleanup(hid_device *handle)
{
	ctrl_general_t general_settings = { 0 };

	general_settings.function = GENERAL_CTRL_NONE;

	if (buddy_write_raw(handle, APP_CODE_CTRL, CTRL_GENERAL, 
		(uint8_t *) &general_settings, sizeof(ctrl_general_t)) != BUDDY_ERROR_OK) {
		return BUDDY_ERROR_GENERAL;
	}

	hid_close(handle);
	hid_exit();

	return BUDDY_ERROR_OK;
}