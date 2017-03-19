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

		Timer0_Init();
    //Timer2_Init();
    
    ADC0_Init();
    UART0_Init();
    debug(("\r\n\r\nUART0_Init passed\r\n"));
    
		//Timer0_Set_Period(1000000);
		//Timer0_Set_Period(10000);
		//Timer0_Set_Period(50000000);
	
    EA = 1;
    
    TLV5630_DAC_Init();
    debug(("TLV5630_DAC_Init passed\r\n"));
		
    while (1) {
        process();
    }
}

