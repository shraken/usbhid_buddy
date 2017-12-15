/** @file main.h
 *  @brief Function prototypes for the main application.
 *
 */

#ifndef _C8051F380_MAIN_H
#define _C8051F380_MAIN_H

#include <compiler_defs.h>
#include <c8051f380.h>
#include <globals.h>

#define TEST_PIN GPIO_P0_0
#define LED_PIN GPIO_P1_6
 
void Delay(void);
void SYSTEMCLOCK_Init (void);
void putchar (char c);

#endif