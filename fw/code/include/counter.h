/**
 * @file counter.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-11-24
 * @brief Tick counter using level or edge based interrupt
 *				 with the INT0/INT1 Configuration of the C8051
 *				 chip.
 *			
 */

#ifndef  _COUNTER_H
#define  _COUNTER_H

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <c8051f3xx.h>
#include "utility.h"
#include "buddy_common.h"

#define RUNTIME_DEFAULT_COUNTER_CONTROL \
	((RUNTIME_COUNTER_CONTROL_ACTIVE_HIGH << RUNTIME_COUNTER_CONTROL_ACTIVE_BITPOS) | \
	(RUNTIME_COUNTER_CONTROL_TRIGGER_EDGE << RUNTIME_COUNTER_CONTROL_TRIGGER_BITPOS))

#define COUNTER_CHANNEL_0 0
#define COUNTER_CHANNEL_1 1

#define COUNTER_ITEM_SIZE sizeof(uint32_t)

/**
 * \enum COUNTER_ERROR_CODE
 * \brief list of error codes that can be returned by counter functions.
 */
typedef enum _COUNTER_ERROR_CODE {
	COUNTER_ERROR_CODE_OK = 0,
	COUNTER_ERROR_CODE_GENERAL_ERROR = -1,
} COUNTER_ERROR_CODE;

int8_t counter_init(uint8_t control, uint8_t chan_mask);
void counter_reset(void);
void counter_pin_init(void);
void counter_enable(void);
void counter_disable(void);
int32_t counter_get_chan0(void);
int32_t counter_get_chan1(void);

void int0_isr (void) __interrupt (INTERRUPT_INT0);
void int1_isr (void) __interrupt (INTERRUPT_INT1);

#endif