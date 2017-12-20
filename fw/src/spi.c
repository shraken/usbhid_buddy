#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <compiler_defs.h>
#include <C8051F380_defs.h>
#include <c8051f3xx.h>
#include <spi.h>
#include <gpio.h>
#include <globals.h>

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
uint8_t SPI_Data_Rx_Array[SPI_MAX_BUFFER_SIZE] = { 0 };
uint8_t SPI_Data_Tx_Array[SPI_MAX_BUFFER_SIZE] = { 0 };

uint8_t bytes_trans;

void spi_init()
{
   SPI0CFG   = 0x50;
   SPI0CN    = 0x0D;
   
   // The equation for SPI0CKR is (SYSCLK/(2*F_SCK_MAX))-1
   SPI0CKR   = (SYSCLK/(2*SPI_CLOCK)) - 1;
}

void spi_array_readwrite(void)
{
    int i;

    for (i = 0; i < bytes_trans; i++) {
        NSSMD0   = 0;
        SPI0DAT  = SPI_Data_Tx_Array[i];
        while (!SPIF);
        SPIF     = 0;
    }

    NSSMD0   = 1;
}

void spi_select(void)
{
	// Use mode 2 with data valid on leading edge but clock idle high
	SPI0CFG = 0x50;
		
	// disable SPI NSS skip
	P1SKIP &= ~(1 << 3);
	
	return;
}