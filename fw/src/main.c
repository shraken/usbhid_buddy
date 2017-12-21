/**
 * @file main.c
 * @brief Main block
 */

#include <compiler_defs.h>
#include <c8051f380.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <main.h>
#include <utility.h>
#include <common.h>
#include <timers.h>
#include <uart.h>
#include <spi.h>
#include <init.h>
#include <adc.h>
#include <gpio.h>
#include <process.h>
#include <tlv563x.h>

#include "c8051f3xx.h"
#include "F3xx_USB0_InterruptServiceRoutine.h"

extern unsigned char String0Desc [4];
extern unsigned char* const STRINGDESCTABLE [];

int x = 10;
int y = 20;
int z = 30;

unsigned int *blah[] = {
    &x,
    &y,
    &z
};

//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------

// required to disable the watchdog timer before the sdcc mcs51 init routines
// run to init global variables otherwise the watchdog will reset processor
// and the main function will never be reached.
//
// http://community.silabs.com/t5/8-bit-MCU-Knowledge-Base/Code-not-executing-to-main/ta-p/110667
void _sdcc_external_startup (void)
{
    // Disable Watchdog timer
    PCA0MD &= ~0x40;
}

void putchar (char c)  {
    if (c == '\n')  {                // check for newline character
        while (!TI0);                 // wait until UART0 is ready to transmit
        TI0 = 0;                      // clear interrupt flag
        SBUF0 = 0x0d;                 // output carriage return command
    }
    
    while (!TI0);                    // wait until UART0 is ready to transmit
    TI0 = 0;                         // clear interrupt flag
    SBUF0 = c;
}

extern uint8_t SPI_Data_Rx_Array[];
extern uint8_t SPI_Data_Tx_Array[];
extern uint8_t bytes_trans;

extern buddy_ctx_t buddy_ctx;

__code firmware_info_t fw_info = {
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
	printf("   version %d.%d.%d \r\n", fw_info.fw_rev_major,
		fw_info.fw_rev_minor, fw_info.fw_rev_tiny);
	printf("   serial id: %lu (%08lx)\r\n", 
		fw_info.serial, fw_info.serial);
	printf("   build datetime: %lu (%08lx)\r\n", 
		fw_info.flash_datetime, fw_info.flash_datetime);
	
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

void contexts_init(void)
{
	buddy_ctx.daq_state         = GENERAL_CTRL_NONE;
	buddy_ctx.m_ctrl_mode       = MODE_CTRL_IMMEDIATE;
	buddy_ctx.m_adc_mode        = RUNTIME_ADC_MODE_SINGLE_ENDED;
	buddy_ctx.m_pwm_mode        = RUNTIME_PWM_MODE_FREQUENCY;
	buddy_ctx.m_pwm_timebase    = RUNTIME_PWM_TIMEBASE_SYSCLK;
	buddy_ctx.m_counter_control = RUNTIME_COUNTER_CONTROL_ACTIVE_HIGH;
	buddy_ctx.m_chan_mask       = 0;
	buddy_ctx.m_resolution      = RESOLUTION_CTRL_HIGH;
	buddy_ctx.m_data_size       = BUDDY_DATA_SIZE_HIGH;
	buddy_ctx.m_chan_number     = 0;
}

void main(void) 
{
    int i;

    system_init();
    contexts_init();

    gpio_init();
    uart_init();
    timer_init();

    spi_init();
    adc_init();

	print_device_info();
    debug(("tlv563x_dac_init passed\r\n"));

	//PCA0MD = 0x00;                      // Disable watchdog timer
	EA = 1;                             // Globally enable interrupts

    tlv563x_dac_init();
	tlv563x_dac_set_power_mode(0);

    while (1) {
        process();
    }
}