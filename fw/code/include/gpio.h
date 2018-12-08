/**
 * @file gpio.h
 * @brief Simple GPIO wrapper for manipulation
 *
 */
 
#ifndef  _GPIO_H_
#define  _GPIO_H_

#include <stdint.h>
#include <compiler_defs.h>
#include <c8051f380.h>
#include <globals.h>

#define LED_STATUS_PIN GPIO_P2_0
#define LED_HEARTBEAT_PIN GPIO_P2_1
#define TEST_STATUS_PIN GPIO_P3_6

#define SPI_DAC_CS_PIN GPIO_P1_3

// error code used globally for all routines
typedef enum _gpio_error_t
{
    GPIO_ERROR_CODE_SUCCESS = 0,
    GPIO_ERROR_CODE_GENERAL_FAIL = -1,
    GPIO_ERROR_CODE_INDEX_OUT_BOUND = -2
} gpio_error_t;

// Encoded pin definition types in an enum
// explicitly specified for clarity
// MSB..LSB
// XX.YYY.ZZZ
//  XX  = don't care
//  YYY = major (P0, P1, etc.)
//  ZZZ = minor is (0..7). 

#define GPIO_MAJOR_MASK 0x38
#define GPIO_MINOR_MASK 0x07

enum _gpio_pin_type
{
    GPIO_P0_0 = 0x00, // 00 000 000
    GPIO_P0_1 = 0x01,
    GPIO_P0_2 = 0x02,
    GPIO_P0_3 = 0x03,
    GPIO_P0_4 = 0x04,
    GPIO_P0_5 = 0x05,
    GPIO_P0_6 = 0x06,
    GPIO_P0_7 = 0x07, // 00 000 111
    
    GPIO_P1_0 = 0x08, // 00 001 000
    GPIO_P1_1 = 0x09,
    GPIO_P1_2 = 0x0A,
    GPIO_P1_3 = 0x0B,
    GPIO_P1_4 = 0x0C,
    GPIO_P1_5 = 0x0D,
    GPIO_P1_6 = 0x0E,
    GPIO_P1_7 = 0x0F, // 00 001 111
    
    GPIO_P2_0 = 0x10, // 00 010 000
    GPIO_P2_1 = 0x11,
    GPIO_P2_2 = 0x12,
    GPIO_P2_3 = 0x13,
    GPIO_P2_4 = 0x14,
    GPIO_P2_5 = 0x15,
    GPIO_P2_6 = 0x16,
    GPIO_P2_7 = 0x17, // 00 010 111
    
    GPIO_P3_0 = 0x18, // 00 011 000
    GPIO_P3_1 = 0x19,
    GPIO_P3_2 = 0x1A,
    GPIO_P3_3 = 0x1B,
    GPIO_P3_4 = 0x1C,
    GPIO_P3_5 = 0x1D,
    GPIO_P3_6 = 0x1E,
    GPIO_P3_7 = 0x1F, // 00 011 111
    
    GPIO_P4_0 = 0x20, // 00 100 000
    GPIO_P4_1 = 0x21,
    GPIO_P4_2 = 0x22,
    GPIO_P4_3 = 0x23,
    GPIO_P4_4 = 0x24,
    GPIO_P4_5 = 0x25,
    GPIO_P4_6 = 0x26,
    GPIO_P4_7 = 0x27, // 00 100 111
};

enum _gpio_pin_mode
{
    GPIO_MODE_OPEN_DRAIN,
    GPIO_MODE_PUSH_PULL
};

enum _gpio_pin_value
{
    GPIO_VALUE_LOW,
    GPIO_VALUE_HIGH
};

enum _gpio_major_type 
{
    GPIO_MAJOR_PORT0,
    GPIO_MAJOR_PORT1,
    GPIO_MAJOR_PORT2,
    GPIO_MAJOR_PORT3,
    GPIO_MAJOR_PORT4
};

enum _gpio_minor_type 
{
    GPIO_MINOR_PIN0,
    GPIO_MINOR_PIN1,
    GPIO_MINOR_PIN2,
    GPIO_MINOR_PIN3,
    GPIO_MINOR_PIN4,
    GPIO_MINOR_PIN5,
    GPIO_MINOR_PIN6,
    GPIO_MINOR_PIN7
};

typedef enum _gpio_pin_type gpio_pin;
typedef enum _gpio_pin_mode gpio_mode;
typedef enum _gpio_pin_value gpio_value;
typedef enum _gpio_major_type gpio_major_pin;
typedef enum _gpio_minor_type gpio_minor_pin;

/** @brief Explicitly initializes the GPIO pins required for
  *         "drive" functionality as open-drain. 
  *
  *  @return error_t enum indicating success or error.
 */
int8_t gpio_init(void);

/** @brief Sets the pin value to high or low state.
  *
  *  @param pin Encoded pin value from gpio_drive_pins.
  *  @param value High or Low enum enumeration.
  *  @return error_t enum indicating success or error.
 */
int8_t gpio_set_pin_value(gpio_pin pin, gpio_value value);

int8_t gpio_set_pin_mode(gpio_pin pin, gpio_mode mode);
uint8_t gpio_get_pin_value(gpio_pin pin);

#endif