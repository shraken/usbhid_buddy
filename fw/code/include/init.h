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

void system_init(void);
void sysclk_init(void);
void port_init(void);
void usb_init(void);
void Delay(void);

#endif