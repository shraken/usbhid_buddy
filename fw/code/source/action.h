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
void process_ctrl_function(uint8_t function);
void process_ctrl_mode_operation(uint8_t mode, uint8_t queue_type);
int process_ctrl_chan_res(uint8_t function, uint8_t chan_mask, uint8_t res);
void process_ctrl_general(uint8_t *p);
void process_ctrl_timing(uint8_t *p);
int process_ctrl();
void process_out();
void process_in();
void process();

#endif