/** @file main.h
 *  @brief Function prototypes for the main application.
 *
 */

#ifndef  _MAIN_H_
#define  _MAIN_H_

#include <compiler_defs.h>
#include <c8051f380.h>
#include <globals.h>

/*
	Firmware Info Structure values.  In future
	and for production define this in the makefile 
	or make modifiable by a binary editor script. 
 */

#define TEST_PIN GPIO_P0_0
#define LED_PIN GPIO_P1_6
 
#define BUDDY_FW_INFO_SERIAL 0x12345678
#define BUDDY_FW_INFO_DATETIME 0x00000000

#define BUDDY_FW_FWREV_INFO_MAJOR 0
#define BUDDY_FW_FWREV_INFO_MINOR 4
#define BUDDY_FW_FWREV_INFO_TINY 1

#define BUDDY_FW_BOOTLREV_INFO_MAJOR 0
#define BUDDY_FW_BOOTLREV_INFO_MINOR 0
#define BUDDY_FW_BOOTLREV_INFO_TINY 0

void main(void);
void putchar (char c);

#endif