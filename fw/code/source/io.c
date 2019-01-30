#include "io.h"

static uint8_t data in_packet_offset = 0;
static uint8_t data codec_byte_offset = 0;

/**
 * @brief initialize the io subsystem.  set the initial USB
 *  in packet offset to 0.
 */
void io_init(void)
{
    P_IN_PACKET_RECORD = &IN_PACKET[BUFFER0_BASE_OFFSET];
	in_packet_offset = 0;
}

/**
 * @brief clear the USB in packet buffer.  set the pointer
 *  for the USB IN buffer to the first buffer.  
 */
void usb_buffer_clear(void)
{
	P_IN_PACKET_RECORD = &IN_PACKET[0];
	
    codec_reset();
    
	in_packet_record_cycle = 0;
	in_packet_offset = 0;
	in_packet_ready = false;
}

/**
 * @brief send response data message to the host system.  copies the
 *  the provided buffer of given length into the USB IN buffer.
 * 
 * @param buffer pointer to the response data source to be written
 *  to the host PC system.
 * @param length the number of bytes to be written in the USB IN packet
 * 
 * @return Void.
 */
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

/**
 * @brief send response error status message to the host system.  copies the
 *  the provided buffer of given length into the USB IN buffer.
 * 
 * @param error_code arbitary signed byte to be sent to the host when
 *  wrapped in a type status message.  
 * 
 * @return Void.
 */
void respond_status(int8_t error_code)
{
		IN_PACKET[BUDDY_APP_CODE_OFFSET] = BUDDY_RESPONSE_TYPE_STATUS;
		IN_PACKET[BUDDY_APP_INDIC_OFFSET] = error_code;
	
		P_IN_PACKET_SEND = &IN_PACKET[0];
		SendPacket(IN_DATA);
		return;
}

/**
 * @brief build an ADC packet
 * @return Void
 */
void build_adc_packet(void)
{
	static uint8_t data encode_count = 0;
	uint8_t data i;
	uint8_t data current_channel = 0;
	uint16_t data value;
	
	#if defined(ADC_TEST)
	static uint16_t count = 0;
	#endif
	
	//P3 = P3 & ~0x40;
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
	  if (buddy_ctx.m_chan_enable[i]) {
			#if defined(ADC_TEST)
			value = count++;
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
		
	  // USB double buffer assignment for future build_adc_packet calls
		if (in_packet_record_cycle) {
			P_IN_PACKET_RECORD = &IN_PACKET[BUFFER0_BASE_OFFSET];
			in_packet_record_cycle = 0;
		} else {
			P_IN_PACKET_RECORD = &IN_PACKET[BUFFER1_BASE_OFFSET];
			in_packet_record_cycle = 1;
		}
				
	  encode_count = 0;
		in_packet_ready = true;
		in_packet_offset = 0;
	}
	
	//P3 = P3 | 0x40;
}

#if 0
void build_counter_packet_new(void)
{
    int32_t counter_chan0;
	int32_t counter_chan1;
    general_packet_t packet;
    int err_code;

    if ((!buddy_ctx.m_chan_enable[COUNTER_CHANNEL_0]) && (!buddy_ctx.m_chan_enable[COUNTER_CHANNEL_1])) {
		// TODO: verbose exit with failure
		return;
	}
    
    if (buddy_ctx.m_chan_enable[COUNTER_CHANNEL_0]) {
		counter_chan0 = counter_get_chan0();
        packet.channels[BUDDY_CHAN_0] = counter_chan0;
	}
	
	if (buddy_ctx.m_chan_enable[COUNTER_CHANNEL_1]) {
		counter_chan1 = counter_get_chan1();
        packet.channels[BUDDY_CHAN_1] = counter_chan1;
	}
    
    //printf("using counter_chan0 = %ld\r\n", counter_chan0);
    //printf("using counter_chan1 = %ld\r\n", counter_chan1);
    
	err_code = codec_encode(P_IN_PACKET_RECORD, &packet);

	if ((buddy_ctx.m_ctrl_mode == MODE_CTRL_IMMEDIATE) || (err_code == CODEC_STATUS_FULL)) {
        //printf("CODEC_STATUS_FULL or MODE_CTRL_IMMEDIATE\r\n");
        
        *(P_IN_PACKET_RECORD + BUDDY_TYPE_OFFSET) = BUDDY_OUT_DATA_ID;
        *(P_IN_PACKET_RECORD + BUDDY_APP_INDIC_OFFSET) = codec_get_encode_count();
        
		P_IN_PACKET_SEND = P_IN_PACKET_RECORD;
		
		if (in_packet_record_cycle) {
			P_IN_PACKET_RECORD = &IN_PACKET[BUFFER0_BASE_OFFSET];
			in_packet_record_cycle = 0;
		} else {
			P_IN_PACKET_RECORD = &IN_PACKET[BUFFER1_BASE_OFFSET];
			in_packet_record_cycle = 1;
		}

        in_packet_ready = true;
	} else if (err_code != CODEC_STATUS_CONTINUE) {
		printf("buddy_send_pwm: err_code = BUDDY_ERROR_CODE_GENERAL\n");
	}    
}
#endif

/**
 * @brief build an counter packet
 * @return Void
 */
void build_counter_packet(void)
{
	static uint8_t data encode_count = 0;
	uint8_t data current_channel = 0;

	int32_t counter_chan0;
	int32_t counter_chan1;
	
	//P3 = P3 & ~0x40;
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
		  ((in_packet_offset + (buddy_ctx.m_data_size * buddy_ctx.m_chan_number)) >= (MAX_REPORT_SIZE - 3))) {
		*(P_IN_PACKET_RECORD + BUDDY_APP_INDIC_OFFSET) = encode_count;
		P_IN_PACKET_SEND = P_IN_PACKET_RECORD;
		
		if (in_packet_record_cycle) {
			P_IN_PACKET_RECORD = &IN_PACKET[BUFFER0_BASE_OFFSET];
			in_packet_record_cycle = 0;
		} else {
			P_IN_PACKET_RECORD = &IN_PACKET[BUFFER1_BASE_OFFSET];
			in_packet_record_cycle = 1;
		}
				
	  encode_count = 0;
		
		in_packet_ready = true;
		in_packet_offset = 0;
	}
	
	//P3 = P3 | 0x40;
}

/**
 * @brief perform a DAC or PWM update cycle.  Enable the USB OUT endpoint buffer
 *  for the next data.
 * @param immediate bool specify if decode should be done single time and the USB
 *  buffer endpoint immediately armed, otherwise continue decoding till reach end
 *  of buffer and then arm the USB endpoint buffer.
 * @return Void.
 */
void execute_out(bool immediate) {
    int decode_status;
    uint8_t i;
    uint32_t value;
    uint8_t *frame;
    general_packet_t packet;
    
    frame = (uint8_t *) &OUT_PACKET[BUFFER0_BASE_OFFSET];
    decode_status = codec_decode(frame, &packet);

    for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
        if (!codec_is_channel_active(i)) {
            continue;
        }
        
        value = packet.channels[i];
        //printf("value = %lu\r\n", value);
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

            //printf("tlv5630 write %lu for channel %bu\n", value, i);
            //printf("execute_out: tlv5630 write %lu for channel %bu\n", value, i);
            tlv563x_write(i, (uint16_t) value);
		} else if (buddy_ctx.daq_state == GENERAL_CTRL_PWM_ENABLE) {
			if (buddy_ctx.m_pwm_mode == RUNTIME_PWM_MODE_FREQUENCY) {
                pwm_set_frequency(i, value);
			} else if (buddy_ctx.m_pwm_mode == RUNTIME_PWM_MODE_DUTY_CYCLE) {
				pwm_set_duty_cycle(i, (uint16_t) value);
			}
		}
    }
    
    if ((decode_status == CODEC_STATUS_FULL) || (immediate == true)) {
        // @todo: this can be removed, the offset is reset to 0 in the decode for full
        codec_set_offset_count(0);
        
        if (buddy_ctx.daq_state == GENERAL_CTRL_DAC_ENABLE) {
			new_dac_packet = 0;
		} else if (buddy_ctx.daq_state == GENERAL_CTRL_DAC_ENABLE) {
			new_pwm_packet = 0;
		}
        
        Enable_Out1();
    }
    
    if (immediate) {
        flag_usb_out = 0;
    }
}