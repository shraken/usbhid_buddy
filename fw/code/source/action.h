/**
 * @file action.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-09-26
 * @brief Handles the incoming configuration and data messages
 *				from the host.  Prepares the configuration response
 *				and sends the data messages back to the host.  
 *			
 */

#ifndef  _ACTION_H_
#define  _ACTION_H_

void respond_data(uint8_t *buffer, uint8_t length);
void respond_status(int8_t error_code);

void disable_all(void);
void ADC_Control_Set(uint8_t ctrl_value);
void ADC_Reference_Set(uint8_t ctrl_value);
void ADC_Configuration_Set(uint8_t ctrl_value);
void build_adc_packet(void);
void build_counter_packet(void);
void execute_out_stream(void);
void execute_out(void);
int8_t process_ctrl_function(ctrl_general_t *p_general);
int8_t process_ctrl_runtime(uint8_t *p);
void process_ctrl_mode_operation(ctrl_general_t *p_general);
int process_ctrl_chan_res(ctrl_general_t *p_general);
int8_t process_ctrl_general(uint8_t *p);
int8_t process_ctrl_timing(uint8_t *p);
int8_t process_ctrl();
void process_out();
void process_in();
void process();

#endif