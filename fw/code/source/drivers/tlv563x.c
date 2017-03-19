#include <spi.h>
#include <stdint.h>
#include <stdio.h>
#include <tlv563x.h>
#include <c8051f3xx.h>

// shraken TODO: make this configurable as build directive
uint8_t tlv563x_resolution = TLV5630_TYPE;

extern unsigned char SPI_Data_Array[];

void TLV5630_write(uint8_t reg_channel, uint16_t reg_value)
{
		SPI_Data_Array[0] = (reg_channel << 4);
		SPI_Data_Array[0] |= ((reg_value & 0x0F00) >> 8);
		SPI_Data_Array[1] = (reg_value & 0xFF);
		
		//debug(("TLV5630_write_block: writing %02bx:%02bx\r\n", SPI_Data_Array[0], SPI_Data_Array[1]));
		SPI_Array_Write();
	
		// Wait until the Write transfer has // finished  
    while (!NSSMD0);
  
		return;
}

//-----------------------------------------------------------------------------
// TLV5630_DAC_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Configures the TLV5630 DAC setting the CTRL0 and CTRL1 register
// values.
//
//-----------------------------------------------------------------------------
void TLV5630_DAC_Init(void)
{
    // CTRL0
		TLV5630_write(REG_CTRL0, 0x00);
    // Timer0_wait(1);
    
    // CTRL1
		TLV5630_write(REG_CTRL1, 0x00);
    // Timer0_wait(1);
	
		TLV5630_DAC_Reset();
	
		return;
}

void TLV5630_DAC_Reset(void)
{
	  int i;
	
		// set all DAC channels to zero outputs
		for (i = REG_DAC_A; i <= REG_DAC_H; i++) {
			TLV5630_write(i, 0);  
    }
}

//-----------------------------------------------------------------------------
// TLV5630_DAC_set_power_mode
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Sets the current global power state to turn ON/OFF the DAC device.  If
//  power_state is TRUE then normal mode, if FALSE then power off device.
//
//-----------------------------------------------------------------------------
void TLV5630_DAC_set_power_mode(uint8_t power_state)
{
		// CTRL0
		if (power_state) {
				TLV5630_write(REG_CTRL0, 0x00);
		} else {
				TLV5630_write(REG_CTRL0, 0x10);
		}
}

void TLV563x_DAC_set_reference(uint8_t ref_mode)
{
		// CTRL0
}