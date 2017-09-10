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
#include <tlv563x.h>
#include <globals.h>
#include <utility.h>

extern unsigned char xdata OUT_PACKET[];
extern unsigned char xdata IN_PACKET[];

extern uint8_t xdata timer0_flag;

extern bit SendPacketBusy;

extern uint16_t xdata adc_results[MAX_ANALOG_INPUTS];
extern uint8_t code adc_mux_ref_tbl[MAX_ANALOG_INPUTS];
extern uint8_t xdata adc_mux_tbl[MAX_ANALOG_INPUTS];
extern uint8_t xdata adc_channel_count;
extern uint8_t xdata adc_complete;
extern uint8_t xdata adc_int_dec_max;
extern uint8_t xdata adc_channel_index;
extern uint8_t xdata adc_int_dec;

extern code firmware_info_t fw_info;

extern void Delay(void);
extern void DelayLong(void);

unsigned char xdata flag_usb_out = 0;
uint8_t new_dac_packet = 0;

uint8_t xdata daq_state;

uint8_t data in_packet_ready = false;
uint8_t data in_packet_offset = 0;

uint8_t data codec_byte_offset = 0;

uint8_t xdata m_trigger = false;
uint8_t xdata m_ctrl_mode = MODE_CTRL_IMMEDIATE;

uint8_t xdata m_adc_control = DEFAULT_ADC0CN;
uint8_t xdata m_adc_ref = DEFAULT_REF0CN;
uint8_t xdata m_adc_cfg = DEFAULT_ADC0CF;
uint8_t xdata m_chan_mask = 0;
uint8_t xdata m_res_mask = 0;
uint8_t xdata m_chan_number;
uint8_t data m_chan_enable[BUDDY_CHAN_LENGTH];

int8_t xdata res_delta = 0;
uint8_t xdata res_shift = 0;

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

void process_dac_stream(void)
{
	static uint8_t decode_count = 0;
	uint8_t i;
	uint16_t value;
	uint8_t count;
	uint8_t *frame;
	
	P3 = P3 & ~0x40;
	count = OUT_PACKET[BUDDY_APP_INDIC_OFFSET];
	//printf("count = %bd\r\n", count);
	//printf("decode_count = %bd\r\n", decode_count);
	
	frame = (uint8_t *) &OUT_PACKET[BUDDY_APP_VALUE_OFFSET];
	
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (m_chan_enable[i]) {
			value = (*(frame + codec_byte_offset) << 8);
			value |= *(frame + codec_byte_offset + 1);
			codec_byte_offset += 2;
				
			//printf("stream packet.channels[%bd] = %u\r\n", i, value);
			TLV563x_write(i, value);
		}
	}
	
	decode_count++;

	// check if the next packet can fit in the packed array
	if (((codec_byte_offset + (2 * m_chan_number)) > (MAX_REPORT_SIZE - 3)) ||
     	 (decode_count >= count))	{
		codec_byte_offset = 0;
		new_dac_packet = 0;
		decode_count = 0;
				 
		Enable_Out1();
	}
	
	P3 = P3 | 0x40;
	
	return;
}

void process_dac()
{
	uint8_t i;
	uint8_t xdata *frame;
	uint16_t value;

	//printf("process_dac()\r\n");

	P3 = P3 & ~0x40;
	frame = (uint8_t *) &OUT_PACKET[BUDDY_APP_VALUE_OFFSET];
	if (m_ctrl_mode == MODE_CTRL_IMMEDIATE) {
		// decode packet, don't process the return code but immediately
		// pull the buffer and send update
		
		for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
			if (m_chan_enable[i]) {
				value = (*(frame + codec_byte_offset) << 8);
				value |= *(frame + codec_byte_offset + 1);
				
				codec_byte_offset += 2;
				//printf("imm channel %bd value = %u\r\n", i, value);
				TLV563x_write(i, value);
			}
		}
		
		codec_byte_offset = 0;
		flag_usb_out = 0;
		Enable_Out1();
		P3 = P3 | 0x40;
	}
}

//void process_ctrl_function(uint8_t function)
void process_ctrl_function(ctrl_general_t *p_general)
{
	debug(("process_ctrl_function()\r\n"));
	
	if (p_general->function == GENERAL_CTRL_DAC_ENABLE) {
		debug(("CTRL_GENERAL = GENERAL_CTRL_DAC_ENABLE\r\n"));
		daq_state = GENERAL_CTRL_DAC_ENABLE;
		ADC0_Disable();
							
		// enable TLV563x SPI DAC by setting power down (PD) register value
		//TLV563x_DAC_set_power_mode(1);
		TLV563x_DAC_Reset();
		m_trigger = false;
	} else if (p_general->function == GENERAL_CTRL_ADC_ENABLE) {
		debug(("CTRL_GENERAL = GENERAL_CTRL_ADC_ENABLE\r\n"));

		in_packet_offset = 0;
		in_packet_ready = false;
		
		ADC0_Enable();
		daq_state = GENERAL_CTRL_ADC_ENABLE;

		// disable TLV563x SPI DAC by setting power down (PD) register value
		TLV563x_DAC_set_power_mode(0);
	} else if (p_general->function == GENERAL_CTRL_NONE) {
		debug(("CTRL_GENERAL = GENERAL_CTRL_NONE\r\n"));
		
		daq_state = GENERAL_CTRL_NONE;					
		ADC0_Disable();
			
		// turn off RX and TX LED
		//gpio_set_pin_value(STATUS_RX_LED_PIN, GPIO_VALUE_HIGH);
		//gpio_set_pin_value(STATUS_TX_LED_PIN, GPIO_VALUE_HIGH);
		txrx_leds_off();
	}
}

//void process_ctrl_mode_operation(uint8_t mode, uint8_t queue_type)
void process_ctrl_mode_operation(ctrl_general_t *p_general)
{
	debug(("process_ctrl_mode_operation\r\n"));
	m_ctrl_mode = p_general->mode;
}

//int process_ctrl_chan_res(uint8_t function, uint8_t chan_mask, uint8_t res)
int process_ctrl_chan_res(ctrl_general_t *p_general)
{
	int i;
	
	debug(("process_ctrl_channel_resolution\r\n"));
	
	m_chan_mask = p_general->channel_mask;

	// get number of channels activated
	m_chan_number = 0;
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (m_chan_mask & (1 << i)) {
			m_chan_enable[i] = 1;
			m_chan_number++;	
		} else {
			m_chan_enable[i] = 0;
		}
	}
	
	if (p_general->function == GENERAL_CTRL_ADC_ENABLE) {
		adc_channel_count = 0;
		adc_channel_index = 0;
			
		// loop through mask setting and copy selective entries from
		// adc_mux_ref_tbl into 
		for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
			if (m_chan_mask & (1 << i)) {
				debug(("process_ctrl_chan_res: activating %d with value %bx\r\n", i, adc_mux_ref_tbl[i]));
				adc_mux_tbl[adc_channel_count] = adc_mux_ref_tbl[i];
				adc_channel_count++;
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
}

/*
void process_ctrl_register(uint8_t *p)
{
		ctrl_register_t *p_registers;
	
		p_registers = (ctrl_register_t *) p;
		debug(("process_ctrl_register():\r\n"));
		debug(("p_registers->adc_ctrl = %bd (0x%bx)\r\n", p_registers->adc_ctrl, p_registers->adc_ctrl));
		debug(("p_registers->adc_ref = %bd (0x%bx)\r\n", p_registers->adc_ref, p_registers->adc_ref));
		debug(("p_registers->adc_cfg = %bd (0x%bx)\r\n", p_registers->adc_cfg, p_registers->adc_cfg));
				
		if (daq_state == GENERAL_CTRL_DAC_ENABLE) {
				TLV563x_write(REG_CTRL0, p_registers->dac_ctrl0);
				TLV563x_write(REG_CTRL1, p_registers->dac_ctrl1);
		} else if (daq_state == GENERAL_CTRL_ADC_ENABLE) {
				ADC0_Set_Control(p_registers->adc_ctrl);
				ADC0_Set_Configuration(p_registers->adc_cfg);
				ADC0_Set_Configuration(p_registers->adc_ref);
		}
}
*/

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
				/*
			  printf("APP_CODE_DAC\r\n");
				for (i = 0; i < 64; i++) {
					printf("%02bx:", OUT_PACKET[i]);
				}
				printf("\r\n");
				*/
			
				if (daq_state == GENERAL_CTRL_DAC_ENABLE) {
					process_dac();
				}
				
				rx_led_toggle();
				break;
						
			case APP_CODE_TRIGGER:
				debug(("APP_CODE_TRIGGER\r\n"));
				m_trigger = true;
				break;
							
			case APP_CODE_INFO:
				debug(("APP_CODE_INFO\r\n"));
				IN_PACKET[1] = BUDDY_RESPONSE_VALID;
				memcpy(&IN_PACKET[2], &fw_info, sizeof(firmware_info_t));
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
		
		//printf("m_ctrl_mode = MODE_CTRL_STREAM and timer0_flag ON\r\n");
		
		// if oneshot enabled and no trigger received  
		// timer elapsed, if DAC mode then pull value from DAC queue
		// and process.
		if ((daq_state == GENERAL_CTRL_DAC_ENABLE) && (new_dac_packet)) {			
			process_dac_stream();
		} 
	}
}

void build_adc_packet(void)
{
	static uint8_t encode_count = 0;
	static uint16_t data channel_value = 0;
	uint8_t data i;
	uint8_t data current_channel = 0;
	uint8_t err_code;
	
	// after IN_PACKET is copied into IN USB EP the in_packet_ready
	// will be set to false
	if (in_packet_ready) {
		return;
	}
	
	P3 = P3 & ~0x40;
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		//if (m_chan_mask & (1 << i)) {
	  if (m_chan_enable[i]) {	
  		#if defined(ADC_TEST)
			IN_PACKET[BUDDY_APP_VALUE_OFFSET + in_packet_offset] = ((channel_value & 0xFF00) >> 8);
			IN_PACKET[BUDDY_APP_VALUE_OFFSET + in_packet_offset + 1] = (channel_value & 0xFF);
			
			channel_value++;
			#else
			IN_PACKET[BUDDY_APP_VALUE_OFFSET + in_packet_offset] = ((adc_results[current_channel] & 0xFF00) >> 8);
			IN_PACKET[BUDDY_APP_VALUE_OFFSET + in_packet_offset + 1] = (adc_results[current_channel] & 0xFF);
			
			current_channel++;
			#endif
			
			in_packet_offset += 2;
		}
	}
	
	encode_count++;
	
	// check if subsequent packet will overflow buffer
	if ((m_ctrl_mode == MODE_CTRL_IMMEDIATE) || 
		  ((in_packet_offset + (2 * m_chan_number)) > (MAX_REPORT_SIZE - 3))) {
		IN_PACKET[BUDDY_APP_INDIC_OFFSET] = encode_count;
				
	  encode_count = 0;
		in_packet_ready = true;
		in_packet_offset = 0;
	}
	P3 = P3 | 0x40;
}

void process_in(void)
{
	static uint8_t xdata in_counter = 0;
	
	if (daq_state == GENERAL_CTRL_ADC_ENABLE) {
		if (adc_complete) {
			//P3 = P3 & ~0x40;
			build_adc_packet();
			adc_complete = 0;
			//P3 = P3 | 0x40;
			//adc_complete = 0;
		}
		
		if (!SendPacketBusy) {
			if (in_packet_ready) {
				in_packet_ready = false;
				IN_PACKET[BUDDY_APP_CODE_OFFSET] = BUDDY_RESPONSE_VALID | (in_counter++ % BUDDY_MAX_COUNTER);
				
				IN_PACKET[BUDDY_TYPE_OFFSET] = IN_DATA;
				SendPacket(IN_DATA);
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
	//process_in();
	process_in();
}