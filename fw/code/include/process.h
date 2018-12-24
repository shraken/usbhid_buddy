/**
 * @file process.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-12-10
 * @brief Handles the incoming configuration and data messages
 *				from the host.  Prepares the configuration response
 *				and sends the data messages back to the host.  
 *			
 */

#ifndef  _PROCESS_H_
#define  _PROCESS_H_

#include <stdint.h>
#include <adc.h>
#include "buddy_common.h"

extern bit SendPacketBusy;
extern uint8_t timer2_flag;
extern unsigned char xdata OUT_PACKET[];
extern unsigned char xdata *P_IN_PACKET_SEND;

extern code firmware_info_t fw_info;

extern int16_t data adc_results[MAX_ANALOG_INPUTS];
extern uint8_t code adc_mux_ref_tbl[MAX_ANALOG_INPUTS];
extern uint8_t adc_mux_tbl_n[MAX_ANALOG_INPUTS];
extern uint8_t adc_mux_tbl_p[MAX_ANALOG_INPUTS];
extern uint8_t adc_channel_count;
extern uint8_t adc_int_dec_max;
extern uint8_t data adc_channel_index;
extern uint8_t adc_int_dec;
extern uint16_t adc_timer_count;

int8_t process_ctrl_function(ctrl_general_t *p_general);
int8_t process_ctrl_runtime(ctrl_runtime_t *p_runtime);
int8_t process_ctrl_chan_res(ctrl_general_t *p_general);
int8_t process_ctrl_general(ctrl_general_t *p);
int8_t process_ctrl_timing(ctrl_timing_t *p);
int8_t process_ctrl();

/** @brief handle the expander options delivered in the general configuration packet.  We
 *				 do any special device initialization and configuration here.
 *	@param p_general pointer to ctrl_general_t structure with general settings from host driver
 *			   with the expander settings embeddeded.  
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl_chan_expander(ctrl_general_t *p_general);

/** @brief handles incoming messages sent by the host driver.  The host driver will send control, DAC, PWM, trigger,
 *				 or info messages.  This function delegates to the correct handler function.
 *  @return 0 on sucess, -1 on error.
 */
void process_out();
void process_in();
void process();

#endif