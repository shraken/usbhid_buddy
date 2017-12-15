/** @file init.h
 *  @brief Function prototypes for the init routines.
 *
 */
 
#ifndef _C8051F380_INIT_H
#define _C8051F380_INIT_H

#include <compiler_defs.h>
#include <c8051f380.h>
#include <globals.h>

void Oscillator_Init(void);
void Port_Init(void);
void USB0_Init (void);
void Delay (void);

#endif