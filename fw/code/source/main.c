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
	//FIRMWARE_INFO_DAC_TYPE_TLV5632,
};

void print_device_info(void)
{
    printf("                                 \r\n");
		printf(" ____            _     _         \r\n");
    printf("| __ ) _   _  __| | __| |_   _   \r\n");
    printf("|  _ \| | | |/ _` |/ _` | | | |  \r\n");
    printf("| |_) | |_| | (_| | (_| | |_| |  \r\n");
    printf("|____/ \__,_|\__,_|\__,_|\__, |  \r\n");
    printf("                         |___/   \r\n");
		printf("   version %bd.%bd.%bd \r\n", fw_info.fw_rev_major,
				fw_info.fw_rev_minor, fw_info.fw_rev_tiny);
		printf("   serial id: %u (%08x)\r\n", fw_info.serial);
		printf("   build datetime: %u (%08x)\r\n", fw_info.flash_datetime);
	
	switch (fw_info.type_dac) {
		case FIRMWARE_INFO_DAC_TYPE_TLV5630:
			printf("   dac type: TLV5630 (12-bit)\r\n");
			break;
		
		case FIRMWARE_INFO_DAC_TYPE_TLV5631:
			printf("   dac type: TLV5631 (10-bit)\r\n");
			break;
		
		case FIRMWARE_INFO_DAC_TYPE_TLV5632:
			printf("   dac type: TLV5632 (8-bit)\r\n");
			break;
		
		case FIRMWARE_INFO_DAC_TYPE_NONE:
		default:
			printf("   dac type: none\r\n");
			break;
	}
	
	printf("\r\n");
}

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

		print_device_info();
		Timer0_Init();

    EA = 1;
    
    TLV563x_DAC_Init();

    debug(("TLV563x_DAC_Init passed\r\n"));

    while (1) {
			process();
	}
}

