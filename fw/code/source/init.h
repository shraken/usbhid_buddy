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
//#define SYS_INT_OSC        0x00        // Select to use internal oscillator
#define SYS_INT_OSC        0x03

#define SYS_EXT_OSC        0x01        // Select to use an external oscillator
#define SYS_4X_DIV_2       0x02

#define ON  1
#define OFF 0

/** @brief This top-level initialization routine calls all support routine.
 *  @return Void.
 */
void system_init(void);

/** @brief Initialize system clock to maximum frequency.
 *  @return Void.
 */
void sysclk_init(void);

/** @brief Routine configure the Crossbar and GPIO ports.
 *  @return Void.
 */
void port_init(void);


/** @brief Initialize USB0. Enable USB0 interrupts. Enable USB0 transceiver.
 *         Enable USB0 with suspend detection
 *  @return Void.
 */
void usb_init(void);

/** @brief Busy wait delay routine used for USB timing.
 *  @return Void.
 */
void Delay(void);

#endif