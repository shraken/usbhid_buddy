/**
 * @file init.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-09-26
 * @brief Device initialization routines.  
 *			
 */

#ifndef _INIT_H
#define _INIT_H

#include <c8051f3xx.h>
#include <F3xx_USB0_InterruptServiceRoutine.h>
#include <F3xx_USB0_ReportHandler.h>
#include <F3xx_USB0_Register.h>
#include "globals.h"
#include "gpio.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------
// USB clock selections (SFR CLKSEL)
#define USB_4X_CLOCK       0x00        // Select 4x clock multiplier, for USB
#define USB_INT_OSC_DIV_2  0x10        // Full Speed
#define USB_EXT_OSC        0x20
#define USB_EXT_OSC_DIV_2  0x30
#define USB_EXT_OSC_DIV_3  0x40
#define USB_EXT_OSC_DIV_4  0x50

// System clock selections (SFR CLKSEL)
#define SYS_INT_OSC        0x00        // Select to use internal oscillator
#define SYS_EXT_OSC        0x01        // Select to use an external oscillator
#define SYS_4X_DIV_2       0x02
#define SYS_4x             0x03

void system_init(void);
void sysclk_init(void);
void port_init(void);
void usb_init(void);
void Delay(void);

#endif