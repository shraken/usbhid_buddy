#include <string.h>
#include <stdio.h>
#include <math.h>
#include <F3xx_USB0_InterruptServiceRoutine.h>
#include <F3xx_USB0_ReportHandler.h>
#include <c8051f3xx.h>
#include <buddy.h>
#include <action.h>
#include <support.h>
#include <timers.h>
#include <adc.h>
#include <gpio.h>
#include <pwm.h>
#include <counter.h>
#include <tlv563x.h>
#include <globals.h>
#include <utility.h>

extern unsigned char xdata OUT_PACKET[];
extern unsigned char xdata IN_PACKET[];

extern unsigned char xdata *P_IN_PACKET_SEND;
extern unsigned char xdata *P_IN_PACKET_RECORD;
extern unsigned char xdata in_packet_record_cycle;

extern uint8_t xdata timer0_flag;

extern bit SendPacketBusy;

extern int16_t data adc_results[MAX_ANALOG_INPUTS];
extern uint8_t code adc_mux_ref_tbl[MAX_ANALOG_INPUTS];
extern uint8_t xdata adc_mux_tbl_n[MAX_ANALOG_INPUTS];
extern uint8_t xdata adc_mux_tbl_p[MAX_ANALOG_INPUTS];
extern uint8_t xdata adc_channel_count;
extern uint8_t data adc_complete;
extern uint8_t xdata adc_int_dec_max;
extern uint8_t data adc_channel_index;
extern uint8_t xdata adc_int_dec;
extern uint16_t xdata adc_timer_count;

extern code firmware_info_t fw_info;

extern void Delay(void);
extern void DelayLong(void);

uint8_t xdata flag_usb_out = 0;
uint8_t new_dac_packet = 0;
uint8_t new_pwm_packet = 0;

uint8_t xdata daq_state;

uint8_t data in_packet_ready = false;
uint8_t data in_packet_offset = 0;

uint8_t data codec_byte_offset = 0;
uint8_t xdata m_ctrl_mode = MODE_CTRL_IMMEDIATE;

uint8_t xdata m_adc_mode = RUNTIME_ADC_MODE_SINGLE_ENDED;
uint8_t xdata m_pwm_mode = RUNTIME_PWM_MODE_FREQUENCY;
uint8_t xdata m_pwm_timebase = RUNTIME_PWM_TIMEBASE_SYSCLK;
uint8_t xdata m_counter_control = RUNTIME_COUNTER_CONTROL_ACTIVE_HIGH;
uint8_t xdata m_adc_control = DEFAULT_ADC0CN;
uint8_t xdata m_adc_ref = DEFAULT_REF0CN;
uint8_t xdata m_adc_cfg = DEFAULT_ADC0CF;
uint8_t data m_chan_mask = 0;
uint8_t data m_resolution = RESOLUTION_CTRL_HIGH;
uint8_t data m_data_size = BUDDY_DATA_SIZE_HIGH;
uint8_t data m_chan_number;
uint8_t data m_chan_enable[BUDDY_CHAN_LENGTH];

void ADC_Control_Set(uint8_t ctrl_value)
{
    m_adc_control = ctrl_value;
}

void ADC_Reference_Set(uint8_t ctrl_value)
{
    m_adc_ref = ctrl_value;
}

void ADC_Configuration_Set(uint8_t ctrl_value)
{	
    m_adc_cfg = ctrl_value;
}

void disable_all(void)
{
	// disable TLV563x SPI DAC
	TLV563x_DAC_set_power_mode(0);
	
	// disable PWM
	pwm_disable();
	
	// disable ADC
	ADC0_Disable();
	
	// disable counter
	counter_disable();
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
		if (m_chan_enable[i]) {
			if (m_resolution == RESOLUTION_CTRL_SUPER) {
				value = (( ((uint32_t) (*(frame + codec_byte_offset)) ) << 24) | 
									 ( ((uint32_t) (*(frame + codec_byte_offset + 1)) ) << 16) | 
									 ( ((uint32_t) (*(frame + codec_byte_offset + 2)) ) << 8) | 
									 (  (uint32_t) (*(frame + codec_byte_offset + 3)) ));
			} else if (m_resolution == RESOLUTION_CTRL_HIGH) {
				value =  ( ((uint32_t) (*(frame + codec_byte_offset)) << 8) | 
								 ( ((uint32_t) (*(frame + codec_byte_offset + 1)) )));
			} else if (m_resolution == RESOLUTION_CTRL_LOW) {
				value = (uint32_t) (*(frame + codec_byte_offset));
			} else {
				return;
			}
			
			if (daq_state == GENERAL_CTRL_DAC_ENABLE) {
				if (m_resolution == RESOLUTION_CTRL_LOW) {
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

				TLV563x_write(i, (uint16_t) value);
			} else if (daq_state == GENERAL_CTRL_PWM_ENABLE) {
				if (m_pwm_mode == RUNTIME_PWM_MODE_FREQUENCY) {
					pwm_set_frequency(i, value);
				} else if (m_pwm_mode == RUNTIME_PWM_MODE_DUTY_CYCLE) {
					pwm_set_duty_cycle(i, (uint16_t) value);
				}
			}
			
			codec_byte_offset += m_data_size;
		}
	}
	
	decode_count++;

	// check if the next packet can fit in the packed array
	if (((codec_byte_offset + (m_data_size * m_chan_number)) > (MAX_REPORT_SIZE - 3)) ||
     	 (decode_count >= count))	{
		codec_byte_offset = 0;
		
		if (daq_state == GENERAL_CTRL_DAC_ENABLE) {
			new_dac_packet = 0;
		} else if (daq_state == GENERAL_CTRL_DAC_ENABLE) {
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
	uint8_t xdata *frame;
	uint32_t value;

	//P3 = P3 & ~0x40;
	frame = (uint8_t *) &OUT_PACKET[BUDDY_APP_VALUE_OFFSET];
	if (m_ctrl_mode == MODE_CTRL_IMMEDIATE) {
		// decode packet, don't process the return code but immediately
		// pull the buffer and send update
		
		for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
			if (m_chan_enable[i]) {
				if (m_resolution == RESOLUTION_CTRL_SUPER) {
					value = (( ((uint32_t) (*(frame + codec_byte_offset)) ) << 24) | 
									 ( ((uint32_t) (*(frame + codec_byte_offset + 1)) ) << 16) | 
									 ( ((uint32_t) (*(frame + codec_byte_offset + 2)) ) << 8) | 
									 ( (uint32_t) (*(frame + codec_byte_offset + 3)) ));
				} else if (m_resolution == RESOLUTION_CTRL_HIGH) {
					value =  ( ((uint32_t) (*(frame + codec_byte_offset)) << 8) | 
									 ( ((uint32_t) (*(frame + codec_byte_offset + 1)) )));
					
				} else if (m_resolution == RESOLUTION_CTRL_LOW) {
					value = (uint32_t) (*(frame + codec_byte_offset));
				}
				
				if (daq_state == GENERAL_CTRL_DAC_ENABLE) {
					if (m_resolution == RESOLUTION_CTRL_LOW) {
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
					
					TLV563x_write(i, (uint16_t) value);
				} else if (daq_state == GENERAL_CTRL_PWM_ENABLE) {
					if (m_pwm_mode == RUNTIME_PWM_MODE_FREQUENCY) {
						pwm_set_frequency(i, value);
					} else if (m_pwm_mode == RUNTIME_PWM_MODE_DUTY_CYCLE) {
						pwm_set_duty_cycle(i, (uint16_t) value);
					}
				}
				
				codec_byte_offset += m_data_size;
			}
		}
		
		codec_byte_offset = 0;
		flag_usb_out = 0;
		Enable_Out1();
		//P3 = P3 | 0x40;
	}
}

void process_ctrl_function(ctrl_general_t *p_general)
{
	debug(("process_ctrl_function()\r\n"));
	
	// check if requested DAQ function value is in boundary
	if ((p_general->function < GENERAL_CTRL_NONE) ||
		  (p_general->function >= GENERAL_CTRL_LENGTH)) {
		daq_state = GENERAL_CTRL_NONE;
	  return;
  } else {
		daq_state = p_general->function;
	}
	
	switch (p_general->function) {
		case GENERAL_CTRL_DAC_ENABLE:
			debug(("CTRL_GENERAL = GENERAL_CTRL_DAC_ENABLE\r\n"));
			ADC0_Disable();
			pwm_disable();
		
			// enable TLV563x SPI DAC by setting power down (PD) register value
			//TLV563x_DAC_set_power_mode(1);
			TLV563x_DAC_Reset();
			break;
		
		case GENERAL_CTRL_ADC_ENABLE:
			debug(("CTRL_GENERAL = GENERAL_CTRL_ADC_ENABLE\r\n"));
			disable_all();
		
			in_packet_offset = 0;
			in_packet_ready = false;
		
			ADC0_Enable();
			break;
		
		case GENERAL_CTRL_PWM_ENABLE:
			debug(("CTRL_GENERAL = GENERAL_CTRL_PWM_ENABLE\r\n"));
			disable_all();
		
			if (pwm_init(m_pwm_mode, m_resolution, m_chan_mask) != PWM_ERROR_CODE_OK) {
				debug(("process_ctrl_function(): pwm_init failed\r\n"));
				return;
			}
		
			if (pwm_set_timebase(m_pwm_timebase) != PWM_ERROR_CODE_OK) {
				debug(("process_ctrl_function(): pwm_set_timebase failed\r\n"));
				return;
			}
			
			pwm_enable();
			break;
		
		case GENERAL_CTRL_COUNTER_ENABLE:
			debug(("CTRL_GENERAL = GENERAL_CTRL_COUNTER_ENABLE\r\n"));
			disable_all();
		
			in_packet_offset = 0;
			in_packet_ready = false;
		
			if (counter_init(m_counter_control, m_chan_mask) != COUNTER_ERROR_CODE_OK) {
				debug(("process_ctrl_function(): counter_init failed\r\n"));
			}
			
			counter_enable();
			break;
			
		case GENERAL_CTRL_NONE:
		default:
			debug(("CTRL_GENERAL = GENERAL_CTRL_NONE\r\n"));
			disable_all();
		
			// turn off RX and TX LED
			txrx_leds_off();
			break;
	}
}

void process_ctrl_mode_operation(ctrl_general_t *p_general)
{
	debug(("process_ctrl_mode_operation\r\n"));
	m_ctrl_mode = p_general->mode;
}

int process_ctrl_chan_res(ctrl_general_t *p_general)
{
	uint8_t i;
	
	debug(("process_ctrl_channel_resolution\r\n"));
	
	m_chan_mask = p_general->channel_mask;
	m_resolution = p_general->resolution;
	
	//printf("m_chan_mask = %bd (%bx)\r\n", m_chan_mask, m_chan_mask);
	
	if (m_resolution == RESOLUTION_CTRL_SUPER) {
		m_data_size = BUDDY_DATA_SIZE_SUPER;
	} else if (m_resolution == RESOLUTION_CTRL_HIGH) {
		m_data_size = BUDDY_DATA_SIZE_HIGH;
	} else if (m_resolution == RESOLUTION_CTRL_LOW) {
		m_data_size = BUDDY_DATA_SIZE_LOW;
	}
	
	// get number of channels activated
	m_chan_number = 0;
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (m_chan_mask & (1 << i)) {
			//printf("process_ctrl_chan_res(): channel %bd activated\r\n", i); 
			m_chan_enable[i] = 1;
			m_chan_number++;	
		} else {
			//printf("process_ctrl_chan_res(): channel %bd not activated\r\n", i);
			m_chan_enable[i] = 0;
		}
	}
	
	if (p_general->function == GENERAL_CTRL_ADC_ENABLE) {
		adc_channel_count = 0;
		adc_channel_index = 0;
		
		// loop through mask setting and copy selective entries from
		// adc_mux_ref_tbl into 
		if (m_adc_mode == RUNTIME_ADC_MODE_SINGLE_ENDED) {
			for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
				if (m_chan_enable[i]) {
					debug(("process_ctrl_chan_res: activating SE %bd with value %bx\r\n", i, adc_mux_ref_tbl[i]));
					adc_mux_tbl_n[adc_channel_count] = adc_mux_ref_tbl[i];
					adc_mux_tbl_p[adc_channel_count] = ADC_GND;
					adc_channel_count++;
				}
			}
		} else if (m_adc_mode == RUNTIME_ADC_MODE_DIFFERENTIAL) {
			for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i = i + 2) {
				if (m_chan_enable[i]) {
					debug(("process_ctrl_chan_res: activating DE %bd with value %bx\r\n", i, adc_mux_ref_tbl[i]));
					adc_mux_tbl_n[adc_channel_count] = adc_mux_ref_tbl[i];
					adc_mux_tbl_p[adc_channel_count] = adc_mux_ref_tbl[i + 1];
					adc_channel_count++;
				}
			}
		}
				
		debug(("adc_channel_count = %bd\r\n", adc_channel_count));
	}
	
	return 0;
}

void process_ctrl_general(uint8_t *p)
{
	ctrl_general_t xdata *p_general;
	
	p_general = (ctrl_general_t *) p;
	debug(("process_ctrl_general():\r\n"));
	debug(("p_general->function = %bd (0x%bx)\r\n", p_general->function, p_general->function));
	debug(("p_general->mode = %bd (0x%bx)\r\n", p_general->mode, p_general->mode));
	debug(("p_general->channel_mask = %bd (0x%bx)\r\n", p_general->channel_mask, p_general->channel_mask));
	debug(("p_general->resolution = %bd (0x%bx)\r\n", p_general->resolution, p_general->resolution));
	
	process_ctrl_mode_operation(p_general);
	process_ctrl_chan_res(p_general);
	process_ctrl_function(p_general);
}

void process_ctrl_runtime(uint8_t *p)
{
	ctrl_runtime_t xdata *p_runtime;
	uint8_t xdata adc_reg_value = 0;
	uint8_t xdata dac_reg_value = 0;
	
	p_runtime = (ctrl_runtime_t *) p;
	debug(("process_ctrl_runtime():\r\n"));
	debug(("p_runtime->dac_power = %bd (0x%bx)\r\n", p_runtime->dac_power, p_runtime->dac_power));
	debug(("p_runtime->dac_ref = %bd (0x%bx)\r\n", p_runtime->dac_ref, p_runtime->dac_ref));
	debug(("p_runtime->adc_ref = %bd (0x%bx)\r\n", p_runtime->adc_ref, p_runtime->adc_ref));
	debug(("p_runtime->adc_gain = %bd (0x%bx)\r\n", p_runtime->adc_gain, p_runtime->adc_gain));
	debug(("p_runtime->pwm_mode = %bd (0x%bx)\r\n", p_runtime->pwm_mode, p_runtime->pwm_mode));
	debug(("p_runtime->pwm_timebase = %bd (0x%bx)\r\n", p_runtime->pwm_timebase, p_runtime->pwm_timebase));
	
	// set TLV563X DAC reference voltage (external, or 1/2 Volt internal)
	switch (p_runtime->dac_ref) {
		case RUNTIME_DAC_REF_EXT:
			debug(("p_runtime->dac_ref = RUNTIME_DAC_REF_EXT\r\n"));
			dac_reg_value |= (TLV5630_REF_MODE_EXT << CTRL0_REFERENCE_0);
			break;
						
		case RUNTIME_DAC_REF_INT_1V:
			debug(("p_runtime->dac_ref = RUNTIME_DAC_REF_INT_1V\r\n"));
			dac_reg_value |= (TLV5630_REF_MODE_INT_1V << CTRL0_REFERENCE_0);
			break;
						
		case RUNTIME_DAC_REF_INT_2V:
			debug(("p_runtime->dac_ref = RUNTIME_DAC_REF_INT_2V\r\n"));
			dac_reg_value |= (TLV5630_REF_MODE_INT_2V << CTRL0_REFERENCE_0);
			break;
						
		default:
			return;
	}
				
	// set TLV563X power mode (ON/OFF)
	if (p_runtime->dac_power == RUNTIME_DAC_POWER_OFF) {
		debug(("p_runtime->dac_power = RUNTIME_DAC_POWER_OFF\r\n"));
		dac_reg_value |= (TLV5630_PWR_MODE_ON << CTRL0_POWER);
	} else if (p_runtime->dac_power == RUNTIME_DAC_POWER_ON) {
		debug(("p_runtime->dac_power = RUNTIME_DAC_POWER_ON\r\n"));
		dac_reg_value &= ~(TLV5630_PWR_MODE_ON << CTRL0_POWER);
	}
			
	debug(("TLV563x_write(): reg_value = %bd (%bx)\r\n", dac_reg_value, dac_reg_value));
		
	// write settings to CTRL0 register (pg. 12)
	TLV563x_write(REG_CTRL0, dac_reg_value);

	m_adc_mode = p_runtime->adc_mode;
	
	switch (p_runtime->adc_ref) {
		case RUNTIME_ADC_REF_EXT:
			debug(("p_runtime->adc_ref = RUNTIME_ADC_REF_EXT\r\n"));
			// REGOVR = 0 (voltage reference specified by REFSL bit)
			adc_reg_value &= ~(1 << ADC_REF0CN_REGOVR);
			// REGSL = 0 (Vref pin used as external voltage reference)
			adc_reg_value &= ~(1 << ADC_REF0CN_REFSL);
			break;
					
		case RUNTIME_ADC_REF_INT:
			debug(("p_runtime->adc_ref = RUNTIME_ADC_REF_INT\r\n"));
			// REGOVR = 1 (internal regulator used as voltage reference)
			adc_reg_value |= (1 << ADC_REF0CN_REGOVR);
			// REFBE = 1 (on-chip reference buffer enabled)
			//adc_reg_value |= (1 << ADC_REF0CN_REFBE);
			// BIASE = 1 (internal bias generator ON)
			adc_reg_value |= (1 << ADC_REF0CN_BIASE);
			break;
						
		case RUNTIME_ADC_REF_VDD:
			debug(("p_runtime->adc_ref = RUNTIME_ADC_REF_VDD\r\n"));
			// REGOVR = 0 (voltage reference specified by REFSL bit)
			adc_reg_value &= ~(1 << ADC_REF0CN_REGOVR);
			// REFSL = 1 (VDD used as a voltage reference )
			adc_reg_value |= (1 << ADC_REF0CN_REFSL);
			break;
						
		default:
			break;
	}
			
	if (p_runtime->adc_gain == RUNTIME_ADC_GAIN_1X) {
		debug(("p_runtime->adc_gain = RUNTIME_ADC_GAIN_1X\r\n"));
		adc_reg_value |= (1 << ADC_REF0CN_REFBGS);
	} else if (p_runtime->adc_gain == RUNTIME_ADC_GAIN_2X) {
		debug(("p_runtime->adc_gain = RUNTIME_ADC_GAIN_2X\r\n"));
		adc_reg_value &= ~(1 << ADC_REF0CN_REFBGS);
	}
				
	ADC0_Set_Reference(adc_reg_value);
	
	m_pwm_mode = p_runtime->pwm_mode;
	m_pwm_timebase = p_runtime->pwm_timebase;
	m_counter_control = p_runtime->counter_control;
}

void process_ctrl_timing(uint8_t *p)
{
	ctrl_timing_t xdata *p_timing;
	
	p_timing = (ctrl_timing_t *) p;
	debug(("process_ct\rl_timing():\r\n"));
	debug(("p_timing->period = %lu (%lx)\r\n", p_timing->period, p_timing->period));
	debug(("p_timing->averaging = %bd\r\n", p_timing->averaging));
	
	// set timer interrupt frequency and number of ADC frames
	// to average over
	Timer0_Set_Period(p_timing->period);
	adc_int_dec_max = p_timing->averaging;
}

int process_ctrl()
{
	uint8_t xdata ctrl_type;

    ctrl_type = OUT_PACKET[BUDDY_APP_INDIC_OFFSET];

	debug(("process_ctrl():\r\n"));
    debug(("ctrl_type = 0x%02bX\r\n", ctrl_type));
	
    // check for which CTRL setting to modify
    switch (ctrl_type) 
    {
        case CTRL_GENERAL:
			debug(("CTRL_GENERAL\r\n"));
			process_ctrl_general( (uint8_t *) &OUT_PACKET[BUDDY_APP_VALUE_OFFSET]);
			break;
				
		case CTRL_RUNTIME:
			debug(("CTRL_RUNTIME\r\n"));
			process_ctrl_runtime( (uint8_t *) &OUT_PACKET[BUDDY_APP_VALUE_OFFSET]);
			break;
				
		case CTRL_TIMING:
			debug(("CTRL_TIMING\r\n"));
			process_ctrl_timing( (uint8_t *) &OUT_PACKET[BUDDY_APP_VALUE_OFFSET]);
			break;

		default:
			return -1;
			break;
    }
    
    //debug(("daq_state = %02bx\r\n", daq_state));
    return 0;
}

void process_out()
{
	uint8_t xdata app_code;
	uint8_t i;
	
	// USB HID OUT message from host has been posted.  Check the
	// header if its a CTRL, DAC, or TRIGGER message and process
    if (flag_usb_out) {
        flag_usb_out = 0;
			
		app_code = OUT_PACKET[BUDDY_APP_CODE_OFFSET];
			
		switch (app_code) {
			case APP_CODE_CTRL:
				debug(("APP_CODE_CTRL\r\n"));
				process_ctrl();
				break;
						
			case APP_CODE_DAC:
				new_dac_packet = 1;
				if (daq_state == GENERAL_CTRL_DAC_ENABLE) {
					//process_dac();
					execute_out();
				}
				
				rx_led_toggle();
				break;
						
			case APP_CODE_PWM:
				//debug(("APP_CODE_PWM\r\n"));
				new_pwm_packet = 1;
				if (daq_state == GENERAL_CTRL_PWM_ENABLE) {
					execute_out();
				}
				
				rx_led_toggle();
				break;
				
			case APP_CODE_TRIGGER:
				debug(("APP_CODE_TRIGGER\r\n"));
				break;
							
			case APP_CODE_INFO:
				debug(("APP_CODE_INFO\r\n"));
				IN_PACKET[BUDDY_APP_CODE_OFFSET] = BUDDY_RESPONSE_VALID;
				memcpy(&IN_PACKET[BUDDY_APP_INDIC_OFFSET], &fw_info, sizeof(firmware_info_t));
				
				P_IN_PACKET_SEND = &IN_PACKET[0];
				SendPacket(IN_DATA);
				break;
						
			default:
				break;
		}

		// if stream mode not enabled then enable the HID OUT endpoint
		// immediately after processing
		if (m_ctrl_mode != MODE_CTRL_STREAM) {
			Enable_Out1();
		} else if (app_code != APP_CODE_DAC) {
			Enable_Out1();
		}
	}
		
	// if stream mode is active, then unload the DAC frame from
	// the data queue and execute the DAC update routine
	if ((m_ctrl_mode == MODE_CTRL_STREAM) && (timer0_flag)) {
		timer0_flag = 0;
		
		if (((daq_state == GENERAL_CTRL_DAC_ENABLE) && (new_dac_packet)) ||
			  ((daq_state == GENERAL_CTRL_PWM_ENABLE) && (new_pwm_packet))) {
			execute_out_stream();		
		}
	}
}

void build_counter_packet(void)
{
	static uint8_t data encode_count = 0;
	uint8_t data i;
	uint8_t data current_channel = 0;
	uint16_t data value;
	
	int32_t counter_chan0;
	int32_t counter_chan1;
	
	P3 = P3 & ~0x40;
	if ((!m_chan_enable[COUNTER_CHANNEL_0]) && (!m_chan_enable[COUNTER_CHANNEL_1])) {
		// TODO: verbose exit with failure
		return;
	}
	
	if (m_chan_enable[COUNTER_CHANNEL_0]) {
		counter_chan0 = counter_get_chan0();

		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET) = ((counter_chan0 & 0xFF000000) >> 24);
		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 1) = ((counter_chan0 & 0x00FF0000) >> 16);
		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 2) = ((counter_chan0 & 0x0000FF00) >> 8);
		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 3) = (counter_chan0 & 0xFF);

		encode_count++;
		in_packet_offset += COUNTER_ITEM_SIZE;
	}
	
	if (m_chan_enable[COUNTER_CHANNEL_1]) {
		counter_chan1 = counter_get_chan1();

		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET) = ((counter_chan1 & 0xFF000000) >> 24);
		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 1) = ((counter_chan1 & 0x00FF0000) >> 16);
		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 2) = ((counter_chan1 & 0x0000FF00) >> 8);
		*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 3) = (counter_chan1 & 0xFF);
		
		encode_count++;
		in_packet_offset += COUNTER_ITEM_SIZE;
	}
	
	// check if subsequent packet will overflow buffer
	if ((m_ctrl_mode == MODE_CTRL_IMMEDIATE) || 
		  ((in_packet_offset + (m_data_size * m_chan_number)) > (MAX_REPORT_SIZE - 3))) {
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
		
		//printf("set in_packet_ready = true\n");
		in_packet_ready = true;
		in_packet_offset = 0;
	}
	
	P3 = P3 | 0x40;
}

void build_adc_packet(void)
{
	static uint8_t data encode_count = 0;
	uint8_t data i;
	uint8_t data current_channel = 0;
	uint16_t data value;
	
	P3 = P3 & ~0x40;
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
	  if (m_chan_enable[i]) {
			#if defined(ADC_TEST)
			value = channel_value++;
			//value = adc_timer_count;
			#else
			value = adc_results[current_channel];
			current_channel++;
			#endif
			
			if (m_resolution == RESOLUTION_CTRL_HIGH) {
				*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET) = ((value & 0xFF00) >> 8);
				*(P_IN_PACKET_RECORD + in_packet_offset + BUDDY_APP_VALUE_OFFSET + 1) = (value & 0xFF);
			} else if (m_resolution == RESOLUTION_CTRL_LOW) {				
				*(P_IN_PACKET_RECORD + BUDDY_APP_VALUE_OFFSET + in_packet_offset) = ((value >> 2) & 0xFF);
			} else {
				return;
			}
			
			in_packet_offset += m_data_size;
		}
	}
	
	encode_count++;
	
	// check if subsequent packet will overflow buffer
	if ((m_ctrl_mode == MODE_CTRL_IMMEDIATE) || 
		  ((in_packet_offset + (m_data_size * m_chan_number)) > (MAX_REPORT_SIZE - 3))) {
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

void process_in(void)
{
	static uint8_t xdata in_counter = 0;
	
	if ((daq_state == GENERAL_CTRL_ADC_ENABLE) ||
     	(daq_state == GENERAL_CTRL_COUNTER_ENABLE)) {
		if (!SendPacketBusy) {
			if (in_packet_ready) {
				in_packet_ready = false;
				*(P_IN_PACKET_SEND + BUDDY_APP_CODE_OFFSET) = BUDDY_RESPONSE_VALID | (in_counter++ % BUDDY_MAX_COUNTER);
				
				//P3 = P3 & ~0x40;
				SendPacket(IN_DATA);
				//P3 = P3 | 0x40;
			}
		}
	}
}

void process()
{
	// Host -> Device message
	// USB OUT
	process_out();
	
	// Device -> Host message
	// USB IN
	process_in();
}