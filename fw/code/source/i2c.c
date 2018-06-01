#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <i2c.h>
#include <globals.h>

#include <compiler_defs.h>
#include <C8051F380_defs.h>

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
uint8_t I2C_Rx_Array_Count = 0;
uint8_t I2C_Tx_Array_Count = 0;

uint8_t I2C_Data_Rx_Array[I2C_MAX_BUFFER_SIZE] = { 0 };
uint8_t I2C_Data_Tx_Array[I2C_MAX_BUFFER_SIZE] = { 0 };

bit SMB_BUSY;
bit SMB_RW;

static bool i2c_initialized = false;
static uint8_t TARGET = 0;

static unsigned long NUM_ERRORS;

unsigned char SMB_DATA_IN;             // Global holder for SMBus data
                                       // All receive data is written here

unsigned char SMB_DATA_OUT;            // Global holder for SMBus data.
                                       // All transmit data is read from here

int8_t i2c_write(uint8_t *buffer, uint16_t len) {
	uint8_t bytes_to_copy;
	
	if (!buffer) {
		return I2C_ERROR_CODE_BAD_MEMORY;
	}
	
	bytes_to_copy = (len >= I2C_MAX_BUFFER_SIZE) ? I2C_MAX_BUFFER_SIZE : len;
	
	while (SMB_BUSY);
	SMB_BUSY = 1;
	
	I2C_Tx_Array_Count = len;
	memcpy(I2C_Data_Tx_Array, buffer, bytes_to_copy);
	
	SMB_RW = 0;
	STA = 1;
	
	return I2C_ERROR_CODE_OK;
}

int8_t i2c_read(uint8_t *buffer, uint16_t len) {
	if (!buffer) {
		return I2C_ERROR_CODE_BAD_MEMORY;
	}
	
	I2C_Rx_Array_Count = (len >= I2C_MAX_BUFFER_SIZE) ? I2C_MAX_BUFFER_SIZE : len;
	
	while (SMB_BUSY);
	SMB_BUSY = 1;
	SMB_RW = 1;
	
	STA = 1;
	while (SMB_BUSY);
	
    // @todo copy i2c rx buffer to buffer
	memcpy(buffer, I2C_Data_Rx_Array, I2C_Rx_Array_Count);
	return I2C_ERROR_CODE_OK;
}

void i2c_wait(void) {
    volatile int i;
    
    /*
    // If slave is holding SDA low because of an improper SMBus reset or error
    while(!SDA) {
        // Provide clock pulses to allow the slave to advance out
        // of its current state. This will allow it to release SDA.
        XBR1 = 0x40;                     // Enable Crossbar
        //SCL = 0;                         // Drive the clock low
        for(i = 0; i < 255; i++);        // Hold the clock low
        
        //SCL = 1;                         // Release the clock
        //while(!SCL);                     // Wait for open-drain
                                       // clock output to rise
        for(i = 0; i < 10; i++);         // Hold the clock high
        XBR1 = 0x00;                     // Disable Crossbar
   }
   */
}

int8_t i2c_init(uint8_t i2c_addr) {
    if (!i2c_initialized) {
        i2c_initialized = true;
        
        TARGET = i2c_addr;
        
        // Timer1 overflow as SMBus clock source
        // Disable slave mode
        // Enable setup and hold time
        // Enable SMBus Free Timeout detect
        // Enable SCL low timeout detect
        SMB0CF = 0x5D;
        
        // Enable SMBus
        SMB0CF |= 0x80;
        
        // Enable the SMBus interrupt
        EIE1 |= 0x01; 
    }
    
	return I2C_ERROR_CODE_OK;
}

void i2c_isr(void) interrupt 7 {
    bit FAIL = 0;                       // Used by the ISR to flag failed
                                        // transfers
    static bit ADDR_SEND = 0;           // Used by the ISR to flag byte
                                        // transmissions as slave addresses
    
    if (ARBLOST == 0)                   // Check for errors
    {
        // Normal operation
        switch (SMB0CN & 0xF0)           // Status vector
        {
            // Master Transmitter/Receiver: START condition transmitted.
            case SMB_MTSTA:
                SMB0DAT = TARGET;          // Load address of the target slave
                SMB0DAT &= 0xFE;           // Clear the LSB of the address for the
                                           // R/W bit
                SMB0DAT |= SMB_RW;         // Load R/W bit
                STA = 0;                   // Manually clear START bit
                ADDR_SEND = 1;
                break;

            // Master Transmitter: Data byte transmitted
            case SMB_MTDB:
                if (ACK)                   // Slave ACK?
                {
                    if (ADDR_SEND)          // If the previous byte was a slave
                    {                       // address,
                        ADDR_SEND = 0;       // Next byte is not a slave address
                  
                        if (SMB_RW == WRITE) // If this transfer is a WRITE,
                        {
                            // send data byte
                            SMB0DAT = SMB_DATA_OUT;
                        }
                        else {}              // If this transfer is a READ,
                                             // proceed with transfer without
                                             // writing to SMB0DAT (switch
                                             // to receive mode)
                    }
                    else                     // If previous byte was not a slave
                    {                        // address,
                        STO = 1;             // Set STO to terminate transfer
                        SMB_BUSY = 0;        // And free SMBus interface
                    }
                }
                else                        // If slave NACK,
                {
                    STO = 1;                // Send STOP condition, followed
                    STA = 1;                // By a START
                    NUM_ERRORS++;           // Indicate error
                }
                break;

            // Master Receiver: byte received
            case SMB_MRDB:
                SMB_DATA_IN = SMB0DAT;     // Store received byte
                SMB_BUSY = 0;              // Free SMBus interface
                ACK = 0;                   // Send NACK to indicate last byte
                                           // of this transfer

                STO = 1;                   // Send STOP to terminate transfer
                break;

            default:
                FAIL = 1;                  // Indicate failed transfer
                                           // and handle at end of ISR
                break;

        } // end switch
    }
    else
    {
        // ARBLOST = 1, error occurred... abort transmission
        FAIL = 1;
    } // end ARBLOST if

    if (FAIL)                           // If the transfer failed,
    {
        SMB0CF &= ~0x80;                 // Reset communication
        SMB0CF |= 0x80;
        STA = 0;
        STO = 0;
        ACK = 0;

        SMB_BUSY = 0;                    // Free SMBus

        FAIL = 0;

        NUM_ERRORS++;                    // Indicate an error occurred
    }

    SI = 0;                             // Clear interrupt flag
	
    return;
}