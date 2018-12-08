#include <stdint.h>
#include <support.h>
#include <gpio.h>
#include <globals.h>
#include <tlv563x.h>
#include <pwm.h>
#include <adc.h>
#include <counter.h>

/**
 * @brief disable all subsystem functions.  Will turn off
 *  the DAC, ADC, PWM, and counter mode subsystem modes if
 *  any of them are active. 
 * 
 */
void disable_all(void)
{
	// disable TLV563x SPI DAC
	tlv563x_disable();
	
	// disable PWM
	pwm_disable();
	
	// disable ADC
	adc_disable();
	
	// disable counter
	counter_disable();
}

/**
 * @brief turn the status TX and RX LEDs off. 
 * 
 */
void txrx_leds_off(void)
{
	gpio_set_pin_value(STATUS_RX_LED_PIN, GPIO_VALUE_HIGH);
	gpio_set_pin_value(STATUS_TX_LED_PIN, GPIO_VALUE_HIGH);
}

/**
 * @brief toggle the RX LED GPIO pin.
 * 
 */
void rx_led_toggle(void)
{
	static uint8_t toggle_state = 0;

	if (toggle_state) {
		gpio_set_pin_value(STATUS_RX_LED_PIN, GPIO_VALUE_LOW);
	} else {
		gpio_set_pin_value(STATUS_RX_LED_PIN, GPIO_VALUE_HIGH);
	}
		
	toggle_state = ~toggle_state;
}

/**
 * @brief toggle the TX LED GPIO pin.
 * 
 */
void tx_led_toggle(void)
{
	static uint8_t toggle_state = 0;

	if (toggle_state) {
		gpio_set_pin_value(STATUS_TX_LED_PIN, GPIO_VALUE_LOW);
	} else {
		gpio_set_pin_value(STATUS_TX_LED_PIN, GPIO_VALUE_HIGH);
	}
		
	toggle_state = ~toggle_state;
}