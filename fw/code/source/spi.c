#include "spi.h"

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
uint8_t SPI_Data_Rx_Array[SPI_MAX_BUFFER_SIZE] = { 0 };
uint8_t SPI_Data_Tx_Array[SPI_MAX_BUFFER_SIZE] = { 0 };

uint8_t bytes_trans;

/**
 * @brief SPI interrupt
 * 
 */
void spi_isr(void) interrupt 6
{
	static unsigned char state = 0;
	static unsigned char array_index = 0;
	
	switch (state) {
		// continue sending
		case 0:
			SPI_Data_Rx_Array[array_index] = SPI0DAT;
			array_index++;
		
			SPI0DAT = SPI_Data_Tx_Array[array_index];
			
			// bytes_trans = 2, 2 -1 = 1
            if (array_index >= (bytes_trans - 1)) {
				state = 1;
            }
		
			break;
		
		// copy off last byte in SPI RX buffer and
		// deselect chip select
		case 1:
			SPI_Data_Rx_Array[array_index] = SPI0DAT;
		
			// De-select the Slave
			NSSMD0 = 1;

			array_index = 0;
			state = 0;
			break;
		
		default:
			state = 0;
			break;
	}
	
	// Clear the SPIF flag
	SPIF = 0;
}

/**
 * @brief Configures SPI0 to use 4-wire Single Master mode. The SPI timing is
 *		  configured for Mode 0,0 (data centered on first edge of clock phase and
 *		  SCK line low in idle state).
 *
 * @return Void.
 */
void spi_init(void)
{
   // set the number of bytes in transcation to zero
   bytes_trans = 0;
	
   SPI0CFG = 0x50;

   SPI0CN    = 0x0D;                   // 4-wire Single Master, SPI enabled

   // SPI clock frequency equation from the datasheet
   SPI0CKR   = (BUDDY_SYSCLK/(2*SPI_CLOCK))-1;

   ESPI0 = 1;                          // Enable SPI interrupts
}

/**
 * @brief Preforms a SPI read/write transaction of length `bytes_trans`.
 *
 * @return Void.
 */
void spi_array_readwrite(void)
{
		//P3 = P3 & ~0x40;

		// Wait until the SPI is free, in case
    // it's already busy
    while (!NSSMD0);                    
	
    NSSMD0 = 0;

    //SPI0DAT = Command;
    SPI0DAT = SPI_Data_Tx_Array[0];
	
    // Wait for SPI transcation to complete
    while (!NSSMD0);
	
		//P3 = P3 | 0x40;
}

/**
 * @brief Configures the DAC CS as the primary SPI chip select.  This is a leftover
 *			  artificat when multiple SPI devices were supported and isn't explictly necessary
 *	      and in future should be revised and removed.
 *
 * @return Void.
 */
void spi_select(void)
{
	// Use mode 2 with data valid on leading edge but clock idle high
	SPI0CFG = 0x50;
		
	// disable SPI NSS skip
	P1SKIP &= ~(1 << 3);
	
	return;
}