#include <stdint.h>
#include <support.h>
#include <gpio.h>
#include <globals.h>
#include <tlv563x.h>
#include <pwm.h>
#include <adc.h>
#include <counter.h>

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

void txrx_leds_off(void)
{
	gpio_set_pin_value(STATUS_RX_LED_PIN, GPIO_VALUE_HIGH);
	gpio_set_pin_value(STATUS_TX_LED_PIN, GPIO_VALUE_HIGH);
}

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