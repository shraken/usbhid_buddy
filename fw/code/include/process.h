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
#include <string.h>
#include <stdio.h>
#include <c8051f3xx.h>
#include <math.h>

#include "F3xx_USB0_InterruptServiceRoutine.h"
#include "F3xx_USB0_ReportHandler.h"
#include "globals.h"
#include "support.h"
#include "timers.h"
#include "adc.h"
#include "init.h"
#include "gpio.h"
#include "pwm.h"
#include "adc.h"
#include "counter.h"
#include "io.h"
#include "utility.h"
#include "poncho.h"
#include "drivers/poncho.h"
#include "drivers/tlv563x.h"
#include "buddy_common.h"
#include "codec.h"

#include "compiler_defs.h"

extern uint8_t SendPacketBusy;
extern unsigned char __xdata OUT_PACKET[];

extern unsigned char __xdata flag_usb_out;
extern uint8_t __data in_packet_ready;
extern uint8_t new_pwm_packet;

extern __code firmware_info_t fw_info;

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