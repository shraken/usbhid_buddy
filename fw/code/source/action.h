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

void ADC_Control_Set(uint8_t ctrl_value);
void ADC_Reference_Set(uint8_t ctrl_value);
void ADC_Configuration_Set(uint8_t ctrl_value);
void build_adc_packet(void);
void execute_out_stream(void);
void execute_out(void);
void process_ctrl_function(ctrl_general_t *p_general);
void process_ctrl_mode_operation(ctrl_general_t *p_general);
int process_ctrl_chan_res(ctrl_general_t *p_general);
void process_ctrl_general(uint8_t *p);
void process_ctrl_timing(uint8_t *p);
int process_ctrl();
void process_out();
void process_in();
void process();

#endif