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

unsigned char* pSMB_DATA_IN;           // Global pointer for SMBus data
                                       // All receive data is written here

unsigned char SMB_SINGLEBYTE_OUT;      // Global holder for single byte writes.

unsigned char* pSMB_DATA_OUT;          // Global pointer for SMBus data.
                                       // All transmit data is read from here

unsigned char SMB_DATA_LEN;            // Global holder for number of bytes
                                       // to send or receive in the current
                                       // SMBus transfer.

unsigned char WORD_ADDR;               // Global holder for the EEPROM word
                                       // address that will be accessed in
                                       // the next transfer

bit SMB_BUSY = 0;
bit SMB_RW;

bit SMB_SENDWORDADDR;                  // When set, this flag causes the ISR
                                       // to send the 8-bit <WORD_ADDR>
                                       // after sending the slave address.
     
bit SMB_RANDOMREAD;                    // When set, this flag causes the ISR
                                       // to send a START signal after sending
                                       // the word address.
                                       // For the 24LC02B EEPROM, a random read
                                       // (a read from a particular address in
                                       // memory) starts as a write then
                                       // changes to a read after the repeated
                                       // start is sent. The ISR handles this
                                       // switchover if the <SMB_RANDOMREAD>
                                       // bit is set.

bit SMB_ACKPOLL;                       // When set, this flag causes the ISR
                                       // to send a repeated START until the
                                       // slave has acknowledged its address
                                       
static bool i2c_initialized = false;
static uint8_t TARGET = 0x20;

static unsigned long NUM_ERRORS;

/**
 * @brief write the buffer with length provided to the i2c slave peripheral
 *  specified in the previous `i2c_init` call.
 * 
 * @param buffer pointer to unsigned byte array buffer of contents to write
 * @param len the number of bytes to write to the i2c slave
 * 
 * @return I2C_ERROR_CODE_OK on success, otherwise I2C_ERROR_CODE_BAD_MEMORY on
 *  bad buffer.
 */
int8_t i2c_write(uint8_t *buffer, uint16_t len) {
	if (!buffer) {
		return I2C_ERROR_CODE_BAD_MEMORY;
	}
	
    while (SMB_BUSY);
	SMB_BUSY = 1;
    
	//bytes_to_copy = (len >= I2C_MAX_BUFFER_SIZE) ? I2C_MAX_BUFFER_SIZE : len;
	I2C_Tx_Array_Count = len;
	memcpy(I2C_Data_Tx_Array, buffer, len);
    
    pSMB_DATA_OUT = (unsigned char *) &I2C_Data_Tx_Array;
    SMB_DATA_LEN = len;
    
	SMB_RW = WRITE;
	STA = 1;
	
	return I2C_ERROR_CODE_OK;
}

/**
 * @brief initialize the i2c master.  this must be called before subsequent calls of
 *  the `i2c_write` and `i2c_read` functions.  This function configures the MCU timers
 *  and SMBUS peripheral and interrupts.  
 * 
 * @param i2c_addr unsigned byte with the i2c slave address being addressed
 * 
 * @return I2C_ERROR_CODE_OK on success. 
 */
int8_t i2c_init(uint8_t i2c_addr) {
    if (!i2c_initialized) {
        i2c_initialized = true;
        
        TARGET = i2c_addr;
        
        // Timer0 overflow as SMBus clock source
        // Disable slave mode
        // Enable setup and hold time
        // Enable SMBus Free Timeout detect
        // Enable SCL low timeout detect
        SMB0CF = 0x5C;
        
        // Enable SMBus
        SMB0CF |= 0x80;
        
        // Enable the SMBus interrupt
        EIE1 |= 0x01; 
    }
    
	return I2C_ERROR_CODE_OK;
}

/**
 * @brief i2c interrupt
 */
void i2c_isr (void) __interrupt (INTERRUPT_SMBUS0) {
   bit FAIL = 0;                       // Used by the ISR to flag failed
                                       // transfers

   static char i;                      // Used by the ISR to count the
                                       // number of data bytes sent or
                                       // received

   static bit SEND_START = 0;          // Send a start

   switch (SMB0CN & 0xF0) {
      // Master Transmitter/Receiver: START condition transmitted.
      case SMB_MTSTA:
         SMB0DAT = TARGET;             // Load address of the target slave
         SMB0DAT &= 0xFE;              // Clear the LSB of the address for the
                                       // R/W bit
         SMB0DAT |= SMB_RW;            // Load R/W bit
         STA = 0;                      // Manually clear START bit
         i = 0;                        // Reset data byte counter
         break;

      // Master Transmitter: Data byte (or Slave Address) transmitted
      case SMB_MTDB:
         if (ACK)                      // Slave Address or Data Byte
         {                             // Acknowledged?
            if (SEND_START)
            {
               STA = 1;
               SEND_START = 0;
               break;
            }

            if (SMB_RW==WRITE)         // Is this transfer a WRITE?
            {

               if (i < SMB_DATA_LEN)   // Is there data to send?
               {
                  // send data byte
                  SMB0DAT = *pSMB_DATA_OUT;

                  // increment data out pointer
                  pSMB_DATA_OUT++;

                  // increment number of bytes sent
                  i++;
               }
               else
               {
                 STO = 1;              // Set STO to terminte transfer
                 SMB_BUSY = 0;         // Clear software busy flag
               }
            }
            else {}                    // If this transfer is a READ,
                                       // then take no action. Slave
                                       // address was transmitted. A
                                       // separate 'case' is defined
                                       // for data byte recieved.
         }
         else                          // If slave NACK,
         {
            if(SMB_ACKPOLL)
            {
               STA = 1;                // Restart transfer
            }
            else
            {
               FAIL = 1;               // Indicate failed transfer
            }                          // and handle at end of ISR
         }
         break;

      // Master Receiver: byte received
      case SMB_MRDB:
         if ( i < SMB_DATA_LEN )       // Is there any data remaining?
         {
            *pSMB_DATA_IN = SMB0DAT;   // Store received byte
            pSMB_DATA_IN++;            // Increment data in pointer
            i++;                       // Increment number of bytes received
            ACK = 1;                   // Set ACK bit (may be cleared later
                                       // in the code)

         }

         if (i == SMB_DATA_LEN)        // This is the last byte
         {
            SMB_BUSY = 0;              // Free SMBus interface
            ACK = 0;                   // Send NACK to indicate last byte
                                       // of this transfer
            STO = 1;                   // Send STOP to terminate transfer
         }

         break;

      default:
         FAIL = 1;                     // Indicate failed transfer
                                       // and handle at end of ISR
         break;
   }

   if (FAIL)                           // If the transfer failed,
   {
      SMB0CF &= ~0x80;                 // Reset communication
      SMB0CF |= 0x80;
      STA = 0;
      STO = 0;
      ACK = 0;

      SMB_BUSY = 0;                    // Free SMBus

      FAIL = 0;
   }

   SI = 0;                             // Clear interrupt flag
}