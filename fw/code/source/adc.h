#ifndef  _ADC_H
#define  _ADC_H

#include <buddy.h>

/*
//#define DEFAULT_ADC0CN 0x02
#define DEFAULT_ADC0CN 0x01

//#define DEFAULT_REF0CN 0x03
#define DEFAULT_REF0CN 0x8F

#define DEFAULT_ADC0CF (((SYSCLK/3000000)-1)<<3)
*/

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

void ADC0_Init(void);
void ADC0_Enable(void);
void ADC0_Disable(void);

void ADC0_Set_Reference(uint8_t value);

#endif