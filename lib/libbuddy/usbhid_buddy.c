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

int buddy_write_packet(hid_device *handle, unsigned char *buffer, int length)
{
	static int count = 0;
	int res;

	res = hid_write(handle, buffer, length);
	if (res < 0) {
		critical(("buddy_write_packet: hid_write call failed, error = %d (%ls) handle = %p\n", 
			res, hid_error(handle), handle));
		return BUDDY_ERROR_CODE_GENERAL;
	}

	return BUDDY_ERROR_CODE_OK;
}

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

int buddy_send_generic(hid_device *handle, general_packet_t *packet, bool streaming, uint8_t type)
{
	int err_code;

	err_code = encode(out_hold_buf, packet);
	encode_count++;

	if ((!streaming) || (err_code == CODEC_STATUS_FULL)) {
		out_hold_buf[BUDDY_TYPE_OFFSET] = BUDDY_OUT_DATA_ID;
		out_hold_buf[BUDDY_APP_CODE_OFFSET] = type;
		out_hold_buf[BUDDY_APP_INDIC_OFFSET] = encode_count;

		if (buddy_write_packet(handle, &out_hold_buf[0], MAX_OUT_SIZE) == BUDDY_ERROR_CODE_GENERAL) {
			critical(("buddy_send_generic: buddy_write_packet call failed\n"));
			return BUDDY_ERROR_CODE_GENERAL;
		}

		encode_count = 0;
		codec_byte_offset = 0;

		return BUDDY_ERROR_CODE_OK;
	} else if (err_code == CODEC_STATUS_CONTINUE) {
		return BUDDY_ERROR_CODE_OK;
	} else {
		printf("buddy_send_generic: err_code = BUDDY_ERROR_CODE_GENERAL\n");
		return BUDDY_ERROR_CODE_GENERAL;
	}
}

int buddy_read_generic(hid_device *handle, general_packet_t *packet, bool streaming)
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
		res = 0;
		while (res == 0) {
			res = buddy_read_packet(handle, in_buf, MAX_IN_SIZE);

			if (res < 0) {
				critical(("buddy_read_adc: could not buddy_read_packet\n"));
				//debugf("res < 0: failed on buddy_read_packet\n");
				return -1;
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
			printf("buddy_send_pwm(): packet->channels[%d] = %d\n", i, packet->channels[i]);

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

int buddy_send_dac(hid_device *handle, general_packet_t *packet, bool streaming)
{
	return buddy_send_generic(handle, packet, streaming, APP_CODE_DAC);
}

int buddy_read_counter(hid_device *handle, general_packet_t *packet, bool streaming)
{
	//printf("buddy_read_counter() entered\n");
	return buddy_read_generic(handle, packet, streaming);
}

int buddy_read_adc(hid_device *handle, general_packet_t *packet, bool streaming)
{
	//printf("buddy_read_adc() entered\n");
	return buddy_read_generic(handle, packet, streaming);
}

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

		if (buddy_write_packet(handle, &out_hold_buf[0], MAX_OUT_SIZE) == BUDDY_ERROR_CODE_GENERAL) {
			critical(("buddy_flush: buddy_write_packet call failed\n"));
			return BUDDY_ERROR_CODE_GENERAL;
		}

		codec_byte_offset = 0;
		encode_count = 0;
	}

	return BUDDY_ERROR_CODE_OK;
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
	int j;
    uint8_t resp_type;
	int8_t err_code;
	buddy_cfg_reg_t cfg_regs[NUMBER_CFG_REG_ENTRIES] = {
		{
		  	.type_indic = CTRL_RUNTIME,
		  	.record_cfg = runtime,
		  	.record_len = sizeof(ctrl_runtime_t),
		},
		{
			.type_indic = CTRL_TIMING,
			.record_cfg = timing,
			.record_len = sizeof(ctrl_timing_t),
		},
		{
			.type_indic = CTRL_GENERAL,
			.record_cfg = general,
			.record_len = sizeof(ctrl_general_t),
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

	if (!(general->function == GENERAL_CTRL_COUNTER_ENABLE)) {
        timing->period = timing->period / buddy_count_channels(general->channel_mask);
    }

	for (i = 0; i < NUMBER_CFG_REG_ENTRIES; i++) {
        for (j = 0; j < BUDDY_MAX_IO_ATTEMPTS; j++) {
		    // request firmware info block
		    if (buddy_write_raw(handle, APP_CODE_CTRL, 
						        	    cfg_regs[i].type_indic, 
							            cfg_regs[i].record_cfg, 
							            cfg_regs[i].record_len) == BUDDY_ERROR_CODE_OK) {
			    short_sleep(100);

                err_code = buddy_get_response(handle, &resp_type, NULL, 0);

                if ((resp_type != BUDDY_ERROR_CODE_OK) || (err_code != BUDDY_RESPONSE_DRV_TYPE_STATUS)) {
                    printf("buddy_configure(): malformed status packet detected\n");
                    printf("resp_type = %d\n", resp_type);
                    printf("err_code = %d\n", err_code);
                    short_sleep(100);
                    continue;
                } else {
                    printf("buddy_configure(): buddy_get_response detected\n");
                    break;
                }
		    } else {
			    critical(("buddy_configure(): failed on buddy_write_raw\n"));
			    return BUDDY_ERROR_CODE_PROTOCOL;
		    }
	    }
	}

	return BUDDY_ERROR_CODE_OK; 
}

int8_t buddy_get_response(hid_device *handle, uint8_t *res_type, uint8_t *buffer, uint8_t length)
{
	uint8_t in_buf[MAX_IN_SIZE] = { 0 };
	uint16_t copy_length;
	int res;
    int j;
	int response_count = 0;

    debug(("buddy_get_response() enter\n"));

    if (!res_type) {
        printf("buddy_get_response(): res_type pointer invalid.\n");
        *res_type = BUDDY_ERROR_CODE_MEMORY;
        return BUDDY_RESPONSE_DRV_TYPE_INTERNAL;
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
                debug(("buddy_get_response(): data type detected\n"));

				if (res > 0) {
					copy_length = (length > (MAX_IN_SIZE - BUDDY_APP_INDIC_OFFSET)) ? (MAX_IN_SIZE - BUDDY_APP_INDIC_OFFSET) : length;
                    debug(("buddy_get_response(): data check pass, copy_length = %d\n", copy_length));
                    memcpy(buffer, &in_buf[BUDDY_APP_INDIC_OFFSET], copy_length);
					*res_type = BUDDY_ERROR_CODE_OK;
                    return BUDDY_RESPONSE_DRV_TYPE_DATA;
				} else {
                    debug(("buddy_get_response(): data check did not pass.\n"));
					*res_type = BUDDY_ERROR_CODE_INVALID;
                    return BUDDY_RESPONSE_DRV_TYPE_DATA;
				}
			} else if (in_buf[BUDDY_APP_CODE_OFFSET] & BUDDY_RESPONSE_TYPE_STATUS) {
				debug(("buddy_get_response(): status packet detected\n"));
                *res_type = in_buf[BUDDY_APP_INDIC_OFFSET];
                return BUDDY_RESPONSE_DRV_TYPE_STATUS;
			} else {
                debug(("buddy_get_response(): invalid error detected\n"));
				*res_type = BUDDY_ERROR_CODE_INVALID;
                return BUDDY_RESPONSE_DRV_TYPE_INTERNAL;
			}

			break;
		}
	}

    debug(("buddy_get_response() exit\n"));
    *res_type = BUDDY_ERROR_CODE_OK;
	return BUDDY_RESPONSE_DRV_TYPE_INTERNAL;
}

int buddy_get_firmware_info(hid_device *handle, firmware_info_t *fw_info)
{
	uint8_t in_buf[MAX_IN_SIZE] = { 0 };
	uint8_t resp_type;
    int res;
	int i;
	int err_code;
	
    printf("buddy_get_firmware_info() entered\n");

    if (!fw_info) {
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

            err_code = buddy_get_response(handle, &resp_type, (uint8_t *) fw_info, sizeof(firmware_info_t));

            if ((resp_type == BUDDY_ERROR_CODE_OK) && (err_code == BUDDY_RESPONSE_DRV_TYPE_DATA)) {  
                // adjust for endian conversion
				fw_info->flash_datetime = swap_uint32(fw_info->flash_datetime);
				fw_info->serial = swap_uint32(fw_info->serial);
				
                return BUDDY_ERROR_CODE_OK;
            } else {
                debug(("buddy_get_firmware_info: buddy_get_response call error\n"));
            }
		} else {
			critical(("buddy_get_firmware_info(): failed on buddy_write_raw\n"));
			return BUDDY_ERROR_CODE_PROTOCOL;
		}
	}

    printf("buddy_get_firmware_info() exit\n");
	return BUDDY_ERROR_CODE_INVALID;
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

    printf("handle = %p\r\n", handle);
    buddy_empty(handle);

	/*
	sprintf(buffer, "handle = %p\r\n", handle);
	debugf(buffer);
	*/

	buddy_get_firmware_info(handle, fw_info);
	return handle;
}

int buddy_cleanup(hid_device *handle, buddy_hid_info_t *hid_info, bool device_disable)
{
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