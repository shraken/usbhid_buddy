/** @file gpio.h
 *  @brief Function prototypes for the GPIO.
 */
 
#ifndef _C8051F380_GPIO_H
#define _C8051F380_GPIO_H

#include <compiler_defs.h>
#include <c8051f380.h>
#include <globals.h>

// prototypes
void gpio_toggle_pin(gpio_pin pin);
error_t gpio_set_pin_value(gpio_pin pin, gpio_value value);
error_t gpio_set_pin_mode(gpio_pin pin, gpio_mode mode);
error_t gpio_activate_channel(gpio_pin pin);
int gpio_get_pin_value(gpio_pin pin);

#endif