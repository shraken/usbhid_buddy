/**
 * @file main.c
 * @brief Main block
 */

#include <compiler_defs.h>
#include <c8051f380.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <common.h>
#include <timers.h>
#include <millis.h>
#include <uart.h>
#include <init.h>
#include <gpio.h>

#include "c8051f3xx.h"
#include "F3xx_USB0_InterruptServiceRoutine.h"

//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
signed char MOUSE_VECTOR;
unsigned char MOUSE_AXIS;
unsigned char MOUSE_BUTTON;

unsigned char IN_PACKET[4];

//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------

// required to disable the watchdog timer before the sdcc mcs51 init routines
// run to init global variables otherwise the watchdog will reset processor
// and the main function will never be reached.
//
// http://community.silabs.com/t5/8-bit-MCU-Knowledge-Base/Code-not-executing-to-main/ta-p/110667
void _sdcc_external_startup (void)
{
    // Disable Watchdog timer
    PCA0MD &= ~0x40;
}

void putchar (char c)  {
    if (c == '\n')  {                // check for newline character
        while (!TI0);                 // wait until UART0 is ready to transmit
        TI0 = 0;                      // clear interrupt flag
        SBUF0 = 0x0d;                 // output carriage return command
    }
    
    while (!TI0);                    // wait until UART0 is ready to transmit
    TI0 = 0;                         // clear interrupt flag
    SBUF0 = c;
}

void blinky_gpio_init(void)
{
    gpio_set_pin_mode(LED_PIN, GPIO_MODE_PUSH_PULL);
    gpio_set_pin_value(LED_PIN, GPIO_VALUE_HIGH);
}

void main(void) 
{
   int pin_state = 0;
   unsigned int main_count = 0;

   Oscillator_Init();                  // Initialize Oscillator
   Port_Init();                        // Initialize Port I/O
   USB0_Init ();

   Timer0_Init();
   UART0_Init();

   EA = 1;

   while (1)
   {
      if (pin_state) {
          gpio_set_pin_value(LED_PIN, GPIO_VALUE_HIGH);
          pin_state = 0;
      } else {
          gpio_set_pin_value(LED_PIN, GPIO_VALUE_LOW);
          pin_state = 1;
      }

      printf("main count = %d\r\n", main_count++);
      millisDelay(100);

      //SendPacket (0);
   }
}