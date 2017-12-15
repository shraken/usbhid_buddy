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

extern bit SendPacketBusy;
extern uint8_t timer0_flag;
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

/** @brief determines function of DAQ device (DAC, ADC, etc.) from the
 *				 p_general->function field and sets up initial required state.
 *	@param p_general pointer to ctrl_general_t structure with general settings from host driver.
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl_function(ctrl_general_t *p_general);

/** @brief determines runtime settings from the p_runtime structure and saves
  *				 off settings into context structure.
 *	@param p_runtime pointer to ctrl_runtime_t with runtime settings from the host driver.
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl_runtime(ctrl_runtime_t *p_runtime);

/** @brief determines resolution and number of channels being requested by the host driver.
 *				 if ADC mode is selected then it prepares the required ADC mux arrays.
 *	@param p_general pointer to ctrl_general_t structure with general settings from host driver.
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl_chan_res(ctrl_general_t *p_general);

/** @brief saves off the requested mode and invokes the channel/resolution and function general handlers.
 *	@param p_general pointer to ctrl_general_t structure with general settings from host driver.
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl_general(ctrl_general_t *p);

/** @brief sets host driver requested timing parameters for streaming mode timer and ADC averaging counts.
 *	@param p_general pointer to ctrl_general_t structure with general settings from host driver.
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl_timing(ctrl_timing_t *p);

/** @brief handles control/configuration messages sent by the host driver.  Checks if a general, runtime, or
 *				 timing request and forwards to to handler function.
 *  @return 0 on sucess, -1 on error.
 */
int8_t process_ctrl();

/** @brief handles incoming messages sent by the host driver.  The host driver will send control, DAC, PWM, trigger,
 *				 or info messages.  This function delegates to the correct handler function.
 *  @return 0 on sucess, -1 on error.
 */
void process_out();

/** @brief Checks to see if any outgoing HID IN packets are queued up and copies them to USB endpoint buffer and
 *				 arms the endpoint.  This can only occur for ADC and counter messages as they are only messages which
 *				 send data from firmware to the host driver.
 *  @return 0 on sucess, -1 on error.
 */
void process_in();

/** @brief Routine that is called an infinite loop by the firmware.  It executes the proess_out and process_in messages.
 *  @return 0 on sucess, -1 on error.
 */
void process();

#endif