#include <main.h>
#include <data.h>
#include <gpio.h>
#include <init.h>
#include <uart.h>
#include <spi.h>
#include <adc.h>
#include <timers.h>
#include <action.h>
#include <tlv563x.h>
#include <23lc1024.h>
#include <utility.h>
#include <stdio.h>
#include <string.h>
#include <c8051f3xx.h>
#include <globals.h>

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

code firmware_info_t fw_info = {
	BUDDY_FW_INFO_SERIAL,
	BUDDY_FW_INFO_DATETIME,
	BUDDY_FW_FWREV_INFO_MAJOR,
	BUDDY_FW_FWREV_INFO_MINOR,
	BUDDY_FW_FWREV_INFO_TINY,
	BUDDY_FW_BOOTLREV_INFO_MAJOR,
	BUDDY_FW_BOOTLREV_INFO_MINOR,
	BUDDY_FW_BOOTLREV_INFO_TINY,
	FIRMWARE_INFO_DAC_TYPE_TLV5630,
	FIRMWARE_INFO_MEM_TYPE_23LC1024,
};

//-----------------------------------------------------------------------------
// Main Routine
//-----------------------------------------------------------------------------
void main(void)
{
    System_Init();
	queue_init();
	gpio_init();
	
    Usb_Init();
    SPI0_Init();
    
    ADC0_Init();
    UART0_Init();

    printf("Buddy DAQ version %bd.%bd.%bd booted!\r\n",
		fw_info.fw_rev_major, 
		fw_info.fw_rev_minor, 
		fw_info.fw_rev_tiny);
	
	Timer0_Init();

    EA = 1;
    
    //TLV563x_DAC_Init();
	SRAM_23LC1024_Init();
	
    debug(("TLV563x_DAC_Init passed\r\n"));
		
    while (1) {
		//gpio_set_pin_value(TEST_STATUS_PIN, GPIO_VALUE_LOW);
		process();
		//gpio_set_pin_value(TEST_STATUS_PIN, GPIO_VALUE_HIGH);
    }
}

