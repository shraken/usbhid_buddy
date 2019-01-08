/**
 * @file support.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-09-26
 * @brief Helper routines for activating status LEDs
 *			
 */

#include <stdint.h>
#include "globals.h"
#include "gpio.h"
#include "pwm.h"
#include "adc.h"
#include "counter.h"
#include "drivers/tlv563x.h"

#ifndef  _SUPPORT_H_
#define  _SUPPORT_H_

#include <stdint.h>
#include "globals.h"
#include "gpio.h"
#include "pwm.h"
#include "adc.h"
#include "counter.h"
#include "drivers/tlv563x.h"

void disable_all(void);

void txrx_leds_off(void);
void rx_led_toggle(void);
void tx_led_toggle(void);

#endif