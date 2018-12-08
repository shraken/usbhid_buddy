/**
 * @file adc.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-09-26
 * @brief Analog to digital converter (ADC) configuration and
 *				data retrieval routines.
 *			
 */

#ifndef  _ADC_H
#define  _ADC_H

#include <stdint.h>
#include <c8051f3xx.h>
#include <buddy.h>

#define ADC_BIT_SIZE 10
#define MAX_ANALOG_INPUTS 8

// CTRL0 register bits, pg. 12
typedef enum _ADC_REF0CN_BITMASK {
    ADC_REF0CN_REFBE = 0,
		ADC_REF0CN_BIASE = 1,
		ADC_REF0CN_TEMPE = 2,
		ADC_REF0CN_REFSL = 3,
		ADC_REF0CN_REGOVR = 4,
		ADC_REF0CN_REFBGS = 7
} ADC_REF0CN;

// C8051F380 48-pin ADC pin macros
// see pg. 60 of C8051F38x datasheet
#define ADC_P0_3 0x11 // 010001b
#define ADC_P0_4 0x12 // 010010b

#define ADC_P1_0 0x15 // 010101b
#define ADC_P1_1 0x13 // 010011b
#define ADC_P1_2 0x14 // 010100b
#define ADC_P1_3 0x16 // 010110b
#define ADC_P1_6 0x17 // 010111b
#define ADC_P1_7 0x18 // 011000b

#define ADC_P2_0 0x00 // 000000b
#define ADC_P2_1 0x01 // 000001b
#define ADC_P2_2 0x02 // 000010b
#define ADC_P2_3 0x03 // 000011b
#define ADC_P2_4 0x19 // 011001b
#define ADC_P2_5 0x04 // 000100b
#define ADC_P2_6 0x05 // 000101b
#define ADC_P2_7 0x1A // 011010b

#define ADC_P3_0 0x06 // 000110b
#define ADC_P3_1 0x07 // 000111b
#define ADC_P3_2 0x1B // 011011b
#define ADC_P3_3 0x1C // 011100b
#define ADC_P3_4 0x08 // 001000b
#define ADC_P3_5 0x09 // 001001b
#define ADC_P3_6 0x1D // 011101b
#define ADC_P3_7 0x0A // 001010b

#define ADC_P4_0 0x0B // 001011b
#define ADC_P4_1 0x20 // 100000b
#define ADC_P4_2 0x21 // 100001b
#define ADC_P4_3 0x0C // 001100b
#define ADC_P4_4 0x0D // 001101b
#define ADC_P4_5 0x0E // 001110b
#define ADC_P4_6 0x0F // 001111b
#define ADC_P4_7 0x22 // 100010b

#define ADC_GND 0x1F  // 011111b
#define ADC_VREF 0x1E // 011110b

int8_t adc_init(void);
int8_t adc_enable(void);
int8_t adc_disable(void);
int8_t adc_set_reference(uint8_t value);

void adc_isr (void) __interrupt (INTERRUPT_ADC0_EOC);

#endif