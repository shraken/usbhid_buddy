/**
 * @file timers.c
 * @brief File containing timer routines for the 
 *  Silicon Labs C8051F380 device type.
 *
 */
 
#include <timers.h>
#include <globals.h>
 
volatile uint32_t _millisCounter = 0;

//-----------------------------------------------------------------------------
// Timer0_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This function configures the Timer0 as a 16-bit timer, interrupt enabled.
// Using the internal osc. at 12MHz with a prescaler of 1:8 and reloading the
// T0 registers with TIMER0_RELOAD_HIGH/LOW it will interrupt and then toggle
// the LED.
//
// Note: The Timer0 uses a 1:48 prescaler.  If this setting changes, the
// TIMER_PRESCALER constant must also be changed.
//-----------------------------------------------------------------------------
void Timer0_Init(void)
{
   TH0 = TIMER0_RELOAD_HIGH;           // Init Timer0 High register
   TL0 = TIMER0_RELOAD_LOW ;           // Init Timer0 Low register
   TMOD = 0x01;                        // Timer0 in 16-bit mode
   //CKCON = 0x02;                     // Timer0 uses a 1:48 prescaler
   CKCON &= 0xF1;                      // Timer0 uses a 1:4 prescaler
   ET0 = 1;                            // Timer0 interrupt enabled
   TCON = 0x10;                        // Timer0 ON
}

//-----------------------------------------------------------------------------
// Timer0_ISR
//-----------------------------------------------------------------------------
//
// Here we process the Timer0 interrupt and toggle the LED
//
//-----------------------------------------------------------------------------
void Timer0_ISR (void) __interrupt (INTERRUPT_TIMER0)
{
   _millisCounter++;
    
   TH0 = TIMER0_RELOAD_HIGH;           // Reinit Timer0 High register
   TL0 = TIMER0_RELOAD_LOW;            // Reinit Timer0 Low register
}