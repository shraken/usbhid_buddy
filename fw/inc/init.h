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

#include <compiler_defs.h>
#include <c8051f380.h>
#include <globals.h>

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