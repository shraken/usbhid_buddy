#ifndef  _GPIO_H_
#define  _GPIO_H_

#include <compiler_defs.h>
#include <C8051F380_defs.h>
#include <globals.h>

#define LED_STATUS_PIN GPIO_P2_0
#define LED_HEARTBEAT_PIN GPIO_P2_1

#define TEST_STATUS_PIN GPIO_P3_6

// prototypes
error_t gpio_init();
error_t gpio_deactivate_all_channels();
error_t gpio_deactivate_channel(gpio_pin pin);
error_t gpio_activate_channel(gpio_pin pin);
error_t gpio_set_pin_value(gpio_pin pin, gpio_value value);
error_t gpio_set_pin_mode(gpio_pin pin, gpio_mode mode);
int gpio_get_pin_value(gpio_pin pin);

#endif