/**
 * @file timers.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-09-26
 * @brief Timer configuration routines.  The timer is used
 *				 as main time keeper for stream operations.  With
 *				 DAC/PWM modes a sample from the frame is output and
 *				 with ADC mode a sample is pulled and copied to the
 *				 buffer.  
 *			
 */

#ifndef  _TIMERS_H_
#define  _TIMERS_H_

#include <stdint.h>
#include <globals.h>
#include <adc.h>
#include <buddy.h>

// Timer 0
#define TIMER0_LOW_PERIOD 21					// 21 nsec
#define TIMER0_HIGH_PERIOD 65535000		// 65.53 sec

#define DEFAULT_TIMER0_HIGH_PERIOD 0xF0
#define DEFAULT_TIMER0_LOW_PERIOD 0x5F

extern buddy_ctx_t buddy_ctx;

extern uint8_t data adc_channel_index;
extern uint8_t adc_channel_count;
extern uint8_t adc_mux_tbl_n[MAX_ANALOG_INPUTS];
extern uint8_t adc_mux_tbl_p[MAX_ANALOG_INPUTS];

extern bit SMB_BUSY;

/** @brief Configures Timer0 with default count values with a 16-bit mode.  The timer
 *				 is by default set to use a SYSCLK/12 reference, timer0 interrupt is enabled,
 *				 and the timer is enabled.
 *  @return Void.
 */
void timer0_init (void);

/** @brief Configure timer1 as smbus/i2c clock source.  Timer1 configured as
 *		   8-bit auto-reload, SYSCLK or SYSCLK/4 as clock source.  Timer1 overflow
 *		   rate configured 3 * SMB_FREQUENCY.  The SCL clock is (1/3) the timer1
 *		   overflow rate (SMB_FREQUENCY).
 *  @return Void.
 */
void timer1_init(void);

/** @brief Configure timer3 as a low timeout detection feature.  Timer 3 configured
 *		   as a 16-bit auto-reload mode, SYSCLK/12 as clock source.  The reload
 *         register is set to 25 msec and is reloaded on interrupt to 25 msec.
 *  @return Void.
 */
void timer3_init(void);

/** @brief Sets the requested period (in nsec) for Timer 0.  A calculation is performed to
 *				 determine if the timer0 clock base needs to be modified from the default SYSCLK/12
 *				 reference and is modified if need be.  The timer is used for stream mode for triggering
 *				 a conversion on the request DAQ function.
 *  @param period the time period (in nsec) that the timer elapses at.
 *  @return Void.
 */
void timer0_set_period(uint32_t period);

#endif