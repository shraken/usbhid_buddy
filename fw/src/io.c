#include <io.h>

static uint8_t __data in_packet_offset = 0;
static uint8_t __data codec_byte_offset = 0;

void io_init(void)
{
    in_packet_offset = 0;
}

void respond_data(uint8_t *buffer, uint8_t length)
{
    IN_PACKET[BUDDY_APP_CODE_OFFSET] = BUDDY_RESPONSE_TYPE_DATA;
	
    if ((buffer) && (length > 0)) {
        memcpy(&IN_PACKET[BUDDY_APP_INDIC_OFFSET], buffer, length);
	}
				
    P_IN_PACKET_SEND = &IN_PACKET[0];
    SendPacket(IN_DATA);
    return;
}

void respond_status(int8_t error_code)
{
    IN_PACKET[BUDDY_APP_CODE_OFFSET] = BUDDY_RESPONSE_TYPE_STATUS;
    IN_PACKET[BUDDY_APP_INDIC_OFFSET] = error_code;
	
    P_IN_PACKET_SEND = &IN_PACKET[0];
    SendPacket(IN_DATA);
    return;
}

void build_adc_packet(void)
{
	static uint8_t __data encode_count = 0;
	uint8_t __data i;
	uint8_t __data current_channel = 0;
	uint16_t __data value;
	
	P3 = P3 & ~0x40;
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
	  if (buddy_ctx.m_chan_enable[i]) {
			#if defined(ADC_TEST)
			value = channel_value++;
			//value = adc_timer_count;
			#else
			value = adc_results[current_channel];
			current_channel++;
			#endif
			
			if (buddy_ctx.m_resolution == RESOLUTION_CTRL_HIGH) {
				*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET) = ((value & 0xFF00) >> 8);
				*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 1) = (value & 0xFF);
			} else if (buddy_ctx.m_resolution == RESOLUTION_CTRL_LOW) {				
				*(P_IN_PACKET_RECORD + BUDDY_APP_VALUE_OFFSET + in_packet_offset) = ((value >> 2) & 0xFF);
			} else {
				return;
			}
			
			in_packet_offset += buddy_ctx.m_data_size;
		}
	}
	
	encode_count++;
	
	// check if subsequent packet will overflow buffer
	if ((buddy_ctx.m_ctrl_mode == MODE_CTRL_IMMEDIATE) || 
		  ((in_packet_offset + (buddy_ctx.m_data_size * buddy_ctx.m_chan_number)) > (MAX_REPORT_SIZE - 3))) {
		*(P_IN_PACKET_RECORD + BUDDY_APP_INDIC_OFFSET) = encode_count;
		P_IN_PACKET_SEND = P_IN_PACKET_RECORD;
		
		if (in_packet_record_cycle) {
			P_IN_PACKET_RECORD = &IN_PACKET[0];
			in_packet_record_cycle = 0;
		} else {
			P_IN_PACKET_RECORD = &IN_PACKET[64];
			in_packet_record_cycle = 1;
		}
				
	  encode_count = 0;
		in_packet_ready = true;
		in_packet_offset = 0;
	}
	
	P3 = P3 | 0x40;
}

void build_counter_packet(void)
{
	static uint8_t __data encode_count = 0;
	uint8_t __data current_channel = 0;

	int32_t counter_chan0;
	int32_t counter_chan1;
	
	P3 = P3 & ~0x40;
	if ((!buddy_ctx.m_chan_enable[COUNTER_CHANNEL_0]) && (!buddy_ctx.m_chan_enable[COUNTER_CHANNEL_1])) {
		// TODO: verbose exit with failure
		return;
	}
	
	if (buddy_ctx.m_chan_enable[COUNTER_CHANNEL_0]) {
		counter_chan0 = counter_get_chan0();

		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET) = ((counter_chan0 & 0xFF000000) >> 24);
		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 1) = ((counter_chan0 & 0x00FF0000) >> 16);
		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 2) = ((counter_chan0 & 0x0000FF00) >> 8);
		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 3) = (counter_chan0 & 0xFF);

		encode_count++;
		in_packet_offset += COUNTER_ITEM_SIZE;
	}
	
	if (buddy_ctx.m_chan_enable[COUNTER_CHANNEL_1]) {
		counter_chan1 = counter_get_chan1();

		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET) = ((counter_chan1 & 0xFF000000) >> 24);
		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 1) = ((counter_chan1 & 0x00FF0000) >> 16);
		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 2) = ((counter_chan1 & 0x0000FF00) >> 8);
		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 3) = (counter_chan1 & 0xFF);
		
		encode_count++;
		in_packet_offset += COUNTER_ITEM_SIZE;
	}
	
	// check if subsequent packet will overflow buffer
	if ((buddy_ctx.m_ctrl_mode == MODE_CTRL_IMMEDIATE) || 
		  ((in_packet_offset + (buddy_ctx.m_data_size * buddy_ctx.m_chan_number)) > (MAX_REPORT_SIZE - 3))) {
		*(P_IN_PACKET_RECORD + BUDDY_APP_INDIC_OFFSET) = encode_count;
		P_IN_PACKET_SEND = P_IN_PACKET_RECORD;
		
		if (in_packet_record_cycle) {
			P_IN_PACKET_RECORD = &IN_PACKET[0];
			in_packet_record_cycle = 0;
		} else {
			P_IN_PACKET_RECORD = &IN_PACKET[64];
			in_packet_record_cycle = 1;
		}
				
	  encode_count = 0;
		
		in_packet_ready = true;
		in_packet_offset = 0;
	}
	
	P3 = P3 | 0x40;
}

void execute_out_stream(void)
{
	static uint8_t decode_count = 0;
	uint8_t i;
	uint32_t value;
	uint8_t count;
	uint8_t *frame;

	//P3 = P3 & ~0x40;
	count = OUT_PACKET[BUDDY_APP_INDIC_OFFSET];
	frame = (uint8_t *) &OUT_PACKET[BUDDY_APP_VALUE_OFFSET];
	
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (buddy_ctx.m_chan_enable[i]) {
			if (buddy_ctx.m_resolution == RESOLUTION_CTRL_SUPER) {
				value = (( ((uint32_t) (*(frame + codec_byte_offset)) ) << 24) | 
									 ( ((uint32_t) (*(frame + codec_byte_offset + 1)) ) << 16) | 
									 ( ((uint32_t) (*(frame + codec_byte_offset + 2)) ) << 8) | 
									 (  (uint32_t) (*(frame + codec_byte_offset + 3)) ));
			} else if (buddy_ctx.m_resolution == RESOLUTION_CTRL_HIGH) {
				value =  ( ((uint32_t) (*(frame + codec_byte_offset)) << 8) | 
								 ( ((uint32_t) (*(frame + codec_byte_offset + 1)) )));
			} else if (buddy_ctx.m_resolution == RESOLUTION_CTRL_LOW) {
				value = (uint32_t) (*(frame + codec_byte_offset));
			} else {
				return;
			}
			
			if (buddy_ctx.daq_state == GENERAL_CTRL_DAC_ENABLE) {
				if (buddy_ctx.m_resolution == RESOLUTION_CTRL_LOW) {
					// if low resolution mode enabled, then shift values
					// up to equivalent native TLV563x size
					switch (fw_info.type_dac) {
						case FIRMWARE_INFO_DAC_TYPE_TLV5630: // 12-bit
							value = value << 4;
							break;
					
						case FIRMWARE_INFO_DAC_TYPE_TLV5631: // 10-bit
							value = value << 2;
							break;
					
						case FIRMWARE_INFO_DAC_TYPE_TLV5632: // 8-bit
						default:
							// do nothing, already in 8-bit
							break;
					}
				}

				tlv563x_write(i, (uint16_t) value);
			} else if (buddy_ctx.daq_state == GENERAL_CTRL_PWM_ENABLE) {
				if (buddy_ctx.m_pwm_mode == RUNTIME_PWM_MODE_FREQUENCY) {
					pwm_set_frequency(i, value);
				} else if (buddy_ctx.m_pwm_mode == RUNTIME_PWM_MODE_DUTY_CYCLE) {
					pwm_set_duty_cycle(i, (uint16_t) value);
				}
			}
			
			codec_byte_offset += buddy_ctx.m_data_size;
		}
	}
	
	decode_count++;

	// check if the next packet can fit in the packed array
	if (((codec_byte_offset + (buddy_ctx.m_data_size * buddy_ctx.m_chan_number)) > (MAX_REPORT_SIZE - 3)) ||
     	 (decode_count >= count))	{
		codec_byte_offset = 0;
		
		if (buddy_ctx.daq_state == GENERAL_CTRL_DAC_ENABLE) {
			new_dac_packet = 0;
		} else if (buddy_ctx.daq_state == GENERAL_CTRL_DAC_ENABLE) {
			new_pwm_packet = 0;
		}
				 
		decode_count = 0;		 
		Enable_Out1();
	}
	
	//P3 = P3 | 0x40;
	return;	
}

void execute_out(void)
{
	uint8_t i;
	uint8_t *frame;
	uint32_t value;

	//P3 = P3 & ~0x40;
	frame = (uint8_t *) &OUT_PACKET[BUDDY_APP_VALUE_OFFSET];
	if (buddy_ctx.m_ctrl_mode == MODE_CTRL_IMMEDIATE) {
		// decode packet, don't process the return code but immediately
		// pull the buffer and send update
		
		for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
			if (buddy_ctx.m_chan_enable[i]) {
				if (buddy_ctx.m_resolution == RESOLUTION_CTRL_SUPER) {
					value = (( ((uint32_t) (*(frame + codec_byte_offset)) ) << 24) | 
									 ( ((uint32_t) (*(frame + codec_byte_offset + 1)) ) << 16) | 
									 ( ((uint32_t) (*(frame + codec_byte_offset + 2)) ) << 8) | 
									 ( (uint32_t) (*(frame + codec_byte_offset + 3)) ));
				} else if (buddy_ctx.m_resolution == RESOLUTION_CTRL_HIGH) {
					value =  ( ((uint32_t) (*(frame + codec_byte_offset)) << 8) | 
									 ( ((uint32_t) (*(frame + codec_byte_offset + 1)) )));
					
				} else if (buddy_ctx.m_resolution == RESOLUTION_CTRL_LOW) {
					value = (uint32_t) (*(frame + codec_byte_offset));
				}
				
				if (buddy_ctx.daq_state == GENERAL_CTRL_DAC_ENABLE) {
					if (buddy_ctx.m_resolution == RESOLUTION_CTRL_LOW) {
						// if low resolution mode enabled, then shift values
						// up to equivalent native TLV563x size
						switch (fw_info.type_dac) {
							case FIRMWARE_INFO_DAC_TYPE_TLV5630: // 12-bit
								value = value << 4;
								break;
					
							case FIRMWARE_INFO_DAC_TYPE_TLV5631: // 10-bit
								value = value << 2;
								break;
					
							case FIRMWARE_INFO_DAC_TYPE_TLV5632: // 8-bit
							default:
								// do nothing, already in 8-bit
								break;
						}	
					}
					
					tlv563x_write(i, (uint16_t) value);
				} else if (buddy_ctx.daq_state == GENERAL_CTRL_PWM_ENABLE) {
					if (buddy_ctx.m_pwm_mode == RUNTIME_PWM_MODE_FREQUENCY) {
						pwm_set_frequency(i, value);
					} else if (buddy_ctx.m_pwm_mode == RUNTIME_PWM_MODE_DUTY_CYCLE) {
						pwm_set_duty_cycle(i, (uint16_t) value);
					}
				}
				
				codec_byte_offset += buddy_ctx.m_data_size;
			}
		}
		
		codec_byte_offset = 0;
		flag_usb_out = 0;
		Enable_Out1();
		//P3 = P3 | 0x40;
	}
}