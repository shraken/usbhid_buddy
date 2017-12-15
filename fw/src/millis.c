/**
 * @file millis.c
 * @brief File containing Millis timekeeper routines for the 
 *  Silicon Labs C8051F380 device type.
 *
 */
 
#include <stdio.h>
#include <stdint.h>
#include <globals.h>
#include <millis.h>

extern uint32_t _millisCounter;

uint32_t millis(void)
{
    return _millisCounter;
}

void millisDelay(uint32_t DelayTime)
{
    static uint32_t delayTimer = 0;
	delayTimer = millis();
	while (millis() - delayTimer < DelayTime);
}

uint32_t get_millis_tick_count(void)
{
    return _millisCounter;
}