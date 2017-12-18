/**
 * @file support.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-09-26
 * @brief Helper routines for activating status LEDs
 *			
 */

#ifndef  _SUPPORT_H_
#define  _SUPPORT_H_

void disable_all(void);

void txrx_leds_off(void);
void rx_led_toggle(void);
void tx_led_toggle(void);

#endif