#include "process.h"

/// set equal to 1 when a new USB HID OUT packet arrives from the host
uint8_t flag_usb_out = 0;

/// set equal to 1 when a new DAC packet frame data arrives over the USB
/// HID OUT packet.
uint8_t new_dac_packet = 0;

/// set equal to 1 when a new PWM packet frame data arrives over the USB
/// HID OUT packet.
uint8_t new_pwm_packet = 0;

/// set equal to 1 when a USB HID IN packet can be sent to the host
uint8_t data in_packet_ready = false;

/// context of the current firmware.  the context is set in an initial
/// configuration step.  The context specifies resolution, mode, and
/// other config parameters.
buddy_ctx_t buddy_ctx;

/** @brief determines function of DAQ device (DAC, ADC, etc.) from the
 *				 p_general->function field and sets up initial required state.
 *	@param p_general pointer to ctrl_general_t structure with general settings from host driver.
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl_function(ctrl_general_t *p_general)
{
	debug(("process_ctrl_function()\r\n"));
	
	// check if requested DAQ function value is in boundary
	if ((p_general->function < GENERAL_CTRL_NONE) ||
		  (p_general->function >= GENERAL_CTRL_LENGTH)) {
		buddy_ctx.daq_state = GENERAL_CTRL_NONE;
	  
		// error: requested function outside bounds
  } else {
		buddy_ctx.daq_state = p_general->function;
	}
	
	switch (p_general->function) {
		case GENERAL_CTRL_DAC_ENABLE:
			debug(("CTRL_GENERAL = GENERAL_CTRL_DAC_ENABLE\r\n"));
			adc_disable();
			pwm_disable();
			tlv563x_enable();
		
			tlv563x_dac_reset();
			break;
		
		case GENERAL_CTRL_ADC_ENABLE:
			debug(("CTRL_GENERAL = GENERAL_CTRL_ADC_ENABLE\r\n"));
			disable_all();
			usb_buffer_clear();
		
			io_init();
			in_packet_ready = false;

			adc_enable();
			break;
		
		case GENERAL_CTRL_PWM_ENABLE:
			debug(("CTRL_GENERAL = GENERAL_CTRL_PWM_ENABLE\r\n"));
			disable_all();
		
			if (pwm_set_timebase(buddy_ctx.m_pwm_timebase) != PWM_ERROR_CODE_OK) {
				debug(("process_ctrl_function(): pwm_set_timebase failed\r\n"));
				return -1;
			}
						
			if (pwm_init(buddy_ctx.m_pwm_mode, buddy_ctx.m_resolution, buddy_ctx.m_chan_mask) != PWM_ERROR_CODE_OK) {
				debug(("process_ctrl_function(): pwm_init failed\r\n"));
				return -1;
			}
			
			pwm_enable();
			break;
		
		case GENERAL_CTRL_COUNTER_ENABLE:
			debug(("CTRL_GENERAL = GENERAL_CTRL_COUNTER_ENABLE\r\n"));
			disable_all();
		
			io_init();
			in_packet_ready = false;
		
			if (counter_init(buddy_ctx.m_counter_control, buddy_ctx.m_chan_mask) != COUNTER_ERROR_CODE_OK) {
				debug(("process_ctrl_function(): counter_init failed\r\n"));
			}
			
			counter_enable();
			break;
			
		default:
		case GENERAL_CTRL_NONE:
			debug(("CTRL_GENERAL = GENERAL_CTRL_NONE\r\n"));
			disable_all();
		
			// turn off RX and TX LED
			txrx_leds_off();
			break;
	}
	
	return 0;
}

/**
 * @brief configure expander hardware boards
 * 
 * @param p_general structure passed from host with expander type, mode
 *  and pin configurations.
 * @return int8_t BUDDY_ERROR_CODE_OK on success, otherwise self describing
 *  error code.
 */
int8_t process_ctrl_chan_expander(ctrl_general_t *p_general) {
	printf("process_ctrl_chan_expander invoked\n");
	
	if (p_general->expander_type != BUDDY_EXPANDER_TYPE_BASE) {
		buddy_ctx.m_expander_type = p_general->expander_type;
		buddy_ctx.m_expander_mode = p_general->expander_mode;
		buddy_ctx.m_expander_pin_state = p_general->expander_pin_state;
	} else {
		return BUDDY_ERROR_CODE_UNKNOWN;
	}
	
	switch (buddy_ctx.m_expander_type) {
		case BUDDY_EXPANDER_TYPE_PONCHO:
			printf("running poncho_init\r\n");
			if (poncho_init() != PONCHO_ERROR_CODE_OK) {
				printf("could not init poncho expander board\n");
				return BUDDY_ERROR_CODE_GENERAL;
			}
			
			printf("running poncho_configure\r\n");
			printf("buddy_ctx.m_expander_mode = %bd\n", buddy_ctx.m_expander_mode);
			printf("buddy_ctx.m_expander_pin_state = %bd\n", buddy_ctx.m_expander_pin_state);
			
			if (poncho_configure(buddy_ctx.m_expander_mode, 
												   buddy_ctx.m_expander_pin_state) != PONCHO_ERROR_CODE_OK) {
			    
					printf("could not configure poncho expander board\n");
			    return BUDDY_ERROR_CODE_GENERAL;
			}
			
			break;
		
		default:
			printf("unknown expander board with type = %d\n",
				  buddy_ctx.m_expander_type);
			return BUDDY_ERROR_CODE_UNKNOWN;
	}
	
	return BUDDY_ERROR_CODE_OK;
}

/** @brief determines resolution and number of channels being requested by the host driver.
 *				 if ADC mode is selected then it prepares the required ADC mux arrays.
 *	@param p_general pointer to ctrl_general_t structure with general settings from host driver.
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl_chan_res(ctrl_general_t *p_general)
{
	uint8_t i;
	
	debug(("process_ctrl_channel_resolution\r\n"));
	
	buddy_ctx.m_chan_mask = p_general->channel_mask;
	buddy_ctx.m_resolution = p_general->resolution;

	if (buddy_ctx.m_resolution == RESOLUTION_CTRL_SUPER) {
		buddy_ctx.m_data_size = BUDDY_DATA_SIZE_SUPER;
	} else if (buddy_ctx.m_resolution == RESOLUTION_CTRL_HIGH) {
		buddy_ctx.m_data_size = BUDDY_DATA_SIZE_HIGH;
	} else if (buddy_ctx.m_resolution == RESOLUTION_CTRL_LOW) {
		buddy_ctx.m_data_size = BUDDY_DATA_SIZE_LOW;
	}
	
	// get number of channels activated
	buddy_ctx.m_chan_number = 0;
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (buddy_ctx.m_chan_mask & (1 << i)) {
			buddy_ctx.m_chan_enable[i] = 1;
			buddy_ctx.m_chan_number++;	
		} else {
			buddy_ctx.m_chan_enable[i] = 0;
		}
	}
	
	if (p_general->function == GENERAL_CTRL_ADC_ENABLE) {
		adc_channel_count = 0;
		adc_channel_index = 0;
		
		// loop through mask setting and copy selective entries from
		// adc_mux_ref_tbl into 
		if (buddy_ctx.m_adc_mode == RUNTIME_ADC_MODE_SINGLE_ENDED) {
			for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
				if (buddy_ctx.m_chan_enable[i]) {
					debug(("process_ctrl_chan_res: activating SE %bd with value %bx\r\n", i, adc_mux_ref_tbl[i]));
					adc_mux_tbl_n[adc_channel_count] = adc_mux_ref_tbl[i];
					adc_mux_tbl_p[adc_channel_count] = ADC_GND;
					adc_channel_count++;
				}
			}
		} else if (buddy_ctx.m_adc_mode == RUNTIME_ADC_MODE_DIFFERENTIAL) {
			for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i = i + 2) {
				if (buddy_ctx.m_chan_enable[i]) {
					debug(("process_ctrl_chan_res: activating DE %bd with value %bx\r\n", i, adc_mux_ref_tbl[i]));
					adc_mux_tbl_n[adc_channel_count] = adc_mux_ref_tbl[i];
					adc_mux_tbl_p[adc_channel_count] = adc_mux_ref_tbl[i + 1];
					adc_channel_count++;
				}
			}
		}
				
		debug(("adc_channel_count = %bd\r\n", adc_channel_count));
	}
	
	// expander check
	process_ctrl_chan_expander(p_general);
	
	return 0;
}

/** @brief saves off the requested mode and invokes the channel/resolution and function general handlers.
 *	@param p_general pointer to ctrl_general_t structure with general settings from host driver.
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl_general(ctrl_general_t *p_general)
{	
	debug(("process_ctrl_general():\r\n"));
	debug(("p_general->function = %bd (0x%bx)\r\n", p_general->function, p_general->function));
	debug(("p_general->mode = %bd (0x%bx)\r\n", p_general->mode, p_general->mode));
	debug(("p_general->channel_mask = %bd (0x%bx)\r\n", p_general->channel_mask, p_general->channel_mask));
	debug(("p_general->resolution = %bd (0x%bx)\r\n", p_general->resolution, p_general->resolution));
	
	buddy_ctx.m_ctrl_mode = p_general->mode;
	
	process_ctrl_chan_res(p_general);
	process_ctrl_function(p_general);
	
	return BUDDY_ERROR_CODE_OK;
}

/** @brief determines runtime settings from the p_runtime structure and saves
  *				 off settings into context structure.
 *	@param p_runtime pointer to ctrl_runtime_t with runtime settings from the host driver.
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl_runtime(ctrl_runtime_t *p_runtime)
{
	uint8_t adc_reg_value = 0;
	uint8_t dac_reg_value = 0;
	
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
			dac_reg_value |= (TLV563X_REF_MODE_EXT << TLV563X_CTRL0_REFERENCE_0);
			break;
						
		case RUNTIME_DAC_REF_INT_1V:
			debug(("p_runtime->dac_ref = RUNTIME_DAC_REF_INT_1V\r\n"));
			dac_reg_value |= (TLV563X_REF_MODE_INT_1V << TLV563X_CTRL0_REFERENCE_0);
			break;
						
		case RUNTIME_DAC_REF_INT_2V:
			debug(("p_runtime->dac_ref = RUNTIME_DAC_REF_INT_2V\r\n"));
			dac_reg_value |= (TLV563X_REF_MODE_INT_2V << TLV563X_CTRL0_REFERENCE_0);
			break;
						
		default:
			return BUDDY_ERROR_CODE_INVALID;
	}
				
	// set TLV563X power mode (ON/OFF)
	if (p_runtime->dac_power == RUNTIME_DAC_POWER_OFF) {
		debug(("p_runtime->dac_power = RUNTIME_DAC_POWER_OFF\r\n"));
		dac_reg_value |= (TLV563X_PWR_MODE_ON << TLV563X_CTRL0_POWER);
	} else if (p_runtime->dac_power == RUNTIME_DAC_POWER_ON) {
		debug(("p_runtime->dac_power = RUNTIME_DAC_POWER_ON\r\n"));
		dac_reg_value &= ~(TLV563X_PWR_MODE_ON << TLV563X_CTRL0_POWER);
	}
			
	debug(("tlv563x_write(): reg_value = %bd (%bx)\r\n", dac_reg_value, dac_reg_value));
		
	// write settings to CTRL0 register (pg. 12)
	tlv563x_write(TLV563X_REG_CTRL0, dac_reg_value);

	buddy_ctx.m_adc_mode = p_runtime->adc_mode;
	
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
			return BUDDY_ERROR_CODE_INVALID;
	}
			
	if (p_runtime->adc_gain == RUNTIME_ADC_GAIN_1X) {
		debug(("p_runtime->adc_gain = RUNTIME_ADC_GAIN_1X\r\n"));
		adc_reg_value |= (1 << ADC_REF0CN_REFBGS);
	} else if (p_runtime->adc_gain == RUNTIME_ADC_GAIN_2X) {
		debug(("p_runtime->adc_gain = RUNTIME_ADC_GAIN_2X\r\n"));
		adc_reg_value &= ~(1 << ADC_REF0CN_REFBGS);
	}
				
	adc_set_reference(adc_reg_value);
	
	buddy_ctx.m_pwm_mode = p_runtime->pwm_mode;
	buddy_ctx.m_pwm_timebase = p_runtime->pwm_timebase;
	buddy_ctx.m_counter_control = p_runtime->counter_control;
	
	return BUDDY_ERROR_CODE_OK;
}

/** @brief sets host driver requested timing parameters for streaming mode timer and ADC averaging counts.
 *	@param p_general pointer to ctrl_general_t structure with general settings from host driver.
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl_timing(ctrl_timing_t *p_timing)
{
	debug(("process_ct\rl_timing():\r\n"));
	debug(("p_timing->period = %lu (%lx)\r\n", p_timing->period, p_timing->period));
	debug(("p_timing->averaging = %bd\r\n", p_timing->averaging));
	
	// set timer interrupt frequency and number of ADC frames
	// to average over
	timer2_set_period(p_timing->period);
	adc_int_dec_max = p_timing->averaging;
	
	return BUDDY_ERROR_CODE_OK;
}

/** @brief handles control/configuration messages sent by the host driver.  Checks if a general, runtime, or
 *				 timing request and forwards to to handler function.
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl()
{
		uint8_t ctrl_type;
		int8_t err_code;
	
    ctrl_type = OUT_PACKET[BUDDY_APP_INDIC_OFFSET];

		debug(("process_ctrl():\r\n"));
    debug(("ctrl_type = 0x%02bX\r\n", ctrl_type));
	
    // check for which CTRL setting to modify
    switch (ctrl_type) 
    {
      case CTRL_GENERAL:
			debug(("CTRL_GENERAL\r\n"));
			err_code = process_ctrl_general( (ctrl_general_t *) &OUT_PACKET[BUDDY_APP_VALUE_OFFSET]);
			break;
				
		case CTRL_RUNTIME:
			debug(("CTRL_RUNTIME\r\n"));
			err_code = process_ctrl_runtime( (ctrl_runtime_t *) &OUT_PACKET[BUDDY_APP_VALUE_OFFSET]);
			break;
				
		case CTRL_TIMING:
			debug(("CTRL_TIMING\r\n"));
			err_code = process_ctrl_timing( (ctrl_timing_t *) &OUT_PACKET[BUDDY_APP_VALUE_OFFSET]);
			break;

		default:
			err_code = BUDDY_ERROR_CODE_INVALID;
			break;
    }
    
    return err_code;
}

/** @brief handles incoming messages sent by the host driver.  The host driver will send control, DAC, PWM, trigger,
 *				 or info messages.  This function delegates to the correct handler function.
 *  @return 0 on sucess, -1 on error.
 */
void process_out()
{
	uint8_t app_code;
	int8_t err_code ;

	// USB HID OUT message from host has been posted.  Check the
	// header if its a CTRL, DAC, or TRIGGER message and process
    if (flag_usb_out) {
        flag_usb_out = 0;
			
		app_code = OUT_PACKET[BUDDY_APP_CODE_OFFSET];
			
		switch (app_code) {
			case APP_CODE_CTRL:
				debug(("APP_CODE_CTRL\r\n"));
				err_code = process_ctrl();
				respond_status(err_code);
				break;
						
			case APP_CODE_DAC:
				new_dac_packet = 1;
				if (buddy_ctx.daq_state == GENERAL_CTRL_DAC_ENABLE) {
					execute_out();
				}
				
				rx_led_toggle();
				break;
						
			case APP_CODE_PWM:
				new_pwm_packet = 1;
				if (buddy_ctx.daq_state == GENERAL_CTRL_PWM_ENABLE) {
					execute_out();
				}
				
				rx_led_toggle();
				break;
				
			case APP_CODE_TRIGGER:
				debug(("APP_CODE_TRIGGER\r\n"));
				break;
							
			case APP_CODE_INFO:
				debug(("APP_CODE_INFO\r\n"));
				respond_data( (uint8_t *) &fw_info, sizeof(firmware_info_t));
				break;
						
			case APP_CODE_RESET:
				debug(("APP_CODE_RESET\r\n"));
			
				PCA0MD  &= ~0x40;
				PCA0L    = 0x00;
				PCA0H    = 0x00;
				PCA0CPL4 = 0xFF;
				PCA0MD  |= 0x40;
				EA = 1;
			
				// force WDT to expire and reset to occur
				while (1) { };
				break;
			
			default:
				break;
		}

		// if stream mode not enabled then enable the HID OUT endpoint
		// immediately after processing
		if (buddy_ctx.m_ctrl_mode != MODE_CTRL_STREAM) {
			Enable_Out1();
		} else if (app_code != APP_CODE_DAC) {
			Enable_Out1();
		}
	}
		
	// if stream mode is active, then unload the DAC frame from
	// the data queue and execute the DAC update routine
	if ((buddy_ctx.m_ctrl_mode == MODE_CTRL_STREAM) && (timer2_flag)) {
		timer2_flag = 0;
		
		if (((buddy_ctx.daq_state == GENERAL_CTRL_DAC_ENABLE) && (new_dac_packet)) ||
			  ((buddy_ctx.daq_state == GENERAL_CTRL_PWM_ENABLE) && (new_pwm_packet))) {
			execute_out_stream();		
		}
	}
}

/** @brief Checks to see if any outgoing HID IN packets are queued up and copies them to USB endpoint buffer and
 *				 arms the endpoint.  This can only occur for ADC and counter messages as they are only messages which
 *				 send data from firmware to the host driver.
 *  @return 0 on sucess, -1 on error.
 */
void process_in(void)
{
	static uint8_t in_counter = 0;
	
	if ((buddy_ctx.daq_state == GENERAL_CTRL_ADC_ENABLE) ||
     	(buddy_ctx.daq_state == GENERAL_CTRL_COUNTER_ENABLE)) {
		if (!SendPacketBusy) {
			if (in_packet_ready) {
				in_packet_ready = false;
				*(P_IN_PACKET_SEND + BUDDY_APP_CODE_OFFSET) = BUDDY_RESPONSE_TYPE_DATA | (in_counter++ % BUDDY_MAX_COUNTER);
				
				//P3 = P3 & ~0x40;
				SendPacket(IN_DATA);
				//P3 = P3 | 0x40;
			}
		}
	}
}

/** @brief Routine that is called an infinite loop by the firmware.  It executes the proess_out and process_in messages.
 *  @return 0 on sucess, -1 on error.
 */
void process()
{
	// Host -> Device message
	// USB OUT
	process_out();
	
	// Device -> Host message
	// USB IN
	process_in();
}