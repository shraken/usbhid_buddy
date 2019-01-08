#ifndef  _MAIN_H_
#define  _MAIN_H_

#include <stdio.h>
#include <string.h>
#include <c8051f3xx.h>
#include "globals.h"
#include "gpio.h"
#include "init.h"
#include "uart.h"
#include "spi.h"
#include "adc.h"
#include "timers.h"
#include "process.h"
#include "utility.h"
#include "pwm.h"
#include "i2c.h"
#include "io.h"
#include "drivers/tlv563x.h"
#include "drivers/tca9555.h"
#include "drivers/poncho.h"

/*
	Firmware Info Structure values.  In future
	and for production define this in the makefile 
	or make modifiable by a binary editor script. 
 */

#include <stdio.h>
#include <string.h>
#include <c8051f3xx.h>
#include "globals.h"
#include "gpio.h"
#include "init.h"
#include "uart.h"
#include "spi.h"
#include "adc.h"
#include "timers.h"
#include "process.h"
#include "utility.h"
#include "pwm.h"
#include "i2c.h"
#include "io.h"
#include "drivers/tlv563x.h"
#include "drivers/tca9555.h"
#include "drivers/poncho.h"

#define BUDDY_FW_INFO_SERIAL 0x12345678
#define BUDDY_FW_INFO_DATETIME 0x00000000

#define BUDDY_FW_FWREV_INFO_MAJOR 0
#define BUDDY_FW_FWREV_INFO_MINOR 4
#define BUDDY_FW_FWREV_INFO_TINY 1

#define BUDDY_FW_BOOTLREV_INFO_MAJOR 0
#define BUDDY_FW_BOOTLREV_INFO_MINOR 0
#define BUDDY_FW_BOOTLREV_INFO_TINY 0

void main(void);

#endif