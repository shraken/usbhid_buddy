#ifndef  _MAIN_H_
#define  _MAIN_H_

/*
	Firmware Info Structure values.  In future
	and for production define this in the makefile 
	or make modifiable by a binary editor script. 
 */

#define BUDDY_FW_INFO_SERIAL 0x12345678
#define BUDDY_FW_INFO_DATETIME 0x00000000

#define BUDDY_FW_FWREV_INFO_MAJOR 0
#define BUDDY_FW_FWREV_INFO_MINOR 4
#define BUDDY_FW_FWREV_INFO_TINY 0

#define BUDDY_FW_BOOTLREV_INFO_MAJOR 0
#define BUDDY_FW_BOOTLREV_INFO_MINOR 0
#define BUDDY_FW_BOOTLREV_INFO_TINY 0

void main(void);

#endif