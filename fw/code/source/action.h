#ifndef  _ACTION_H_
#define  _ACTION_H_

void ADC_Control_Set(uint8_t ctrl_value);
void ADC_Reference_Set(uint8_t ctrl_value);
void ADC_Configuration_Set(uint8_t ctrl_value);
void update_dac(general_packet_t *packet);
void build_adc_packet(void);
void process_adc_stream(void);
void process_dac_stream(void);
void process_dac();
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