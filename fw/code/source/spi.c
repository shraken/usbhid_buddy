#include <compiler_defs.h>
#include <C8051F380_defs.h>
#include <spi.h>
#include <stdio.h>
#include <stdlib.h>
#include <globals.h>

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

unsigned char SPI_Data = 0x00;
unsigned char SPI_Data_Array[MAX_BUFFER_SIZE] = {0};

unsigned char Error_Flag = 0;
unsigned char Command = 0x00;

//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SPI_ISR
//-----------------------------------------------------------------------------
//
// Handles all error checks and single-byte writes.
//
// Note: SPI_WRITE_ARRAY is not handled by this ISR in order to take
// advantage of double-buffering (checking the TXBMT flag) using polling.
//
//
// Typical Write:
//
//              | 1st sent | 2nd sent | 3rd sent |   ...    | last sent |
//              ---------------------------------------------------------
//  Master NSSv | Command  |   Data1  |   Data2  |   ...    |   DataN   |  NSS^
//  Slave       |   N/A    |    N/A   |    N/A   |   ...    |    N/A    |
//
// Typical Read:
//
//              | 1st sent | 2nd sent | 3rd sent |   ...    | last sent |
//              ---------------------------------------------------------
//  Master NSSv | Command  |   dummy  |   dummy  |   ...    |   dummy   |  NSS^
//  Slave       |   N/A    |   Data1  |   Data2  |   ...    |   DataN   |
//-----------------------------------------------------------------------------
void SPI_ISR (void) interrupt 6
{
   static unsigned char array_index = 0;
   static char state = 0;

   if (WCOL == 1)
   {
      // Write collision occurred
      WCOL = 0;                        // Clear the write collision flag
      Error_Flag = 1;
   }
   else
   {
      if (SPI0DAT == ERROR_OCCURRED)
      {
         // This example recognizes when an error occurs, but does not include
         // any error handling.  The transfer can be aborted or rescheduled,
         // if desired.
         Error_Flag = 1;
      }

      // When the Master enters the ISR, the SPIF flag should be set from
      // sending the Command byte.  This ISR handles the remaining steps of the
      // SPI transfer process.
      // <state> == 0: writing or reading 1 byte of data
      // <state> == 1: for READ commands (first time, only a dummy byte is
      //               sent but the second time, the data must be read from
      //               SPI0DAT)
      // <state> == 2: NSS = 1 to end the transfer, final byte read
      //
      // Note: SPI_WRITE_BUFFER is not handled here because it's done in
      // polled mode
      if (state == 0)
      {
         switch (Command)
         {
            case SPI_WRITE:
               SPI0DAT = SPI_Data;

               state = 2;              // Advance to the final state (only
                                       // writing one byte)

               break;

            case SPI_READ:
               SPI0DAT = 0xFF;         // Send a dummy byte so the Slave can
                                       // send the data

               state = 2;              // Advance to the final state (only
                                       // reading one byte)

               break;

            case SPI_READ_BUFFER:
               array_index = 0;        // Clear the data counter

               SPI0DAT = 0xFF;         // Send a dummy byte so the Slave can
                                       // start sending the data

               state = 1;              // Advance to the next state where the
                                       // data can be received
                                       // The data from the slave is not
                                       // available until after the second
                                       // transfer is completed.
                                       // The dummy byte allows the slave to
                                       // send data, since the Master controls
                                       // SCK.

               break;

            default:
               state = 2;              // Any errors in the Command parsing
                                       // should go to state 2 where NSSMD0
                                       // is de-asserted
         }
      }
      else if (state == 1)             // This state is for READ_ARRAY
      {                                // commands where the data must be read
                                       // after the first dummy byte is sent
         switch (Command)
         {
            case SPI_READ_BUFFER:
               SPI_Data_Array[array_index] = SPI0DAT;
               SPI0DAT = 0xFF;

               array_index++;

               if (array_index == (MAX_BUFFER_SIZE-1))
               {
                  state = 2;
               }

               break;
            default:
               state = 2;              // Any errors in the Command parsing
                                       // should go to state 2 where NSSMD0
                                       // is de-asserted
         }
      }
      else if (state == 2)
      {
         switch (Command)
         {
            case SPI_READ:
               SPI_Data = SPI0DAT;     // Read the data from the slave

               break;

            case SPI_READ_BUFFER:
               SPI_Data_Array[array_index] = SPI0DAT; // Read the last data
                                                      // without sending a
                                                      // dummy byte

               break;
         }

         NSSMD0 = 1;                   // De-select the Slave

         state = 0;                    // Reset the state
      }

      SPIF = 0;                        // Clear the SPIF flag
   }
}

//-----------------------------------------------------------------------------
// SPI0_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Configures SPI0 to use 4-wire Single Master mode. The SPI timing is
// configured for Mode 0,0 (data centered on first edge of clock phase and
// SCK line low in idle state).
//
//-----------------------------------------------------------------------------
void SPI0_Init(void)
{
   //SPI0CFG   = 0x70;                   // Enable the SPI as a Master
   //                                    // CKPHA = '0', CKPOL = '0'
   // 
	 SPI0CFG = 0x50;
  
	 SPI0CN    = 0x0D;                   // 4-wire Single Master, SPI enabled

   // SPI clock frequency equation from the datasheet
   SPI0CKR   = (SYSCLK/(2*SPI_CLOCK))-1;

   ESPI0 = 1;                          // Enable SPI interrupts
}

//-----------------------------------------------------------------------------
// SPI_Byte_Write
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Note: SPI_Data must contain the data to be sent before calling this
// function.
//
// Writes a single byte to the SPI Slave.  The slave does not respond to this
// command, so the command consists of:
//
// Command = SPI_WRITE
// Length = 1 byte of command, 1 byte of data
//
//-----------------------------------------------------------------------------
void SPI_Byte_Write (void)
{
   while (!NSSMD0);                    // Wait until the SPI is free, in case
                                       // it's already busy

   NSSMD0 = 0;

   Command = SPI_WRITE;

   SPI0DAT = Command;

   // The rest of this command will be handled by the SPI ISR, which will
   // trigger when SPIF is set from sending the Command
}

//-----------------------------------------------------------------------------
// SPI_Byte_Read
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Note: SPI_Data will contain the data received after calling this function.
//
// Reads a single byte from the SPI Slave.  The command consists of:
//
// Command = SPI_READ
// Length = 1 byte of command, 1 byte of data
//
//-----------------------------------------------------------------------------
void SPI_Byte_Read (void)
{
   while (!NSSMD0);                    // Wait until the SPI is free, in case
                                       // it's already busy

   NSSMD0 = 0;

   Command = SPI_READ;

   SPI0DAT = Command;

   // The rest of this command will be handled by the SPI ISR, which will
   // trigger when SPIF is set from sending the Command
}

//-----------------------------------------------------------------------------
// SPI_Array_Write
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Note: SPI_Data_Array must contain the data to be sent before calling this
// function.
//
// Writes an array of values of size MAX_BUFFER_SIZE to the SPI Slave.  The
// command consists of:
//
// Command = SPI_WRITE_BUFFER
// Length = 1 byte of command, MAX_BUFFER_SIZE bytes of data
//
// Note: Polled mode is used for this function in order to buffer the data
// being sent using the TXBMT flag.
//
//-----------------------------------------------------------------------------
void SPI_Array_Write (void)
{
   unsigned char array_index;

   while (!NSSMD0);                    // Wait until the SPI is free, in case
                                       // it's already busy

   ESPI0 = 0;                          // Disable SPI interrupts

   NSSMD0 = 0;
    
   for (array_index = 0; array_index < MAX_BUFFER_SIZE; array_index++)
   {
      SPI0DAT = SPI_Data_Array[array_index]; // Load the data into the buffer
      while (TXBMT != 1)               // Wait until the data is moved into
      {                                // the XMIT buffer
      }
   }
   SPIF = 0;
   while (SPIF != 1)                   // Wait until the last byte of the
   {                                   // data reaches the Slave
   }
   SPIF = 0;

   NSSMD0 = 1;                         // Diable the Slave

   ESPI0 = 1;                          // Re-enable SPI interrupts
}

//-----------------------------------------------------------------------------
// SPI_Array_Read
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Note: SPI_Data_Array will contain the data received after calling this
// function.
//
// Reads a single byte from the SPI Slave.  The command consists of:
//
// Command = SPI_READ_BUFFER
// Length = 1 byte of command, MAX_BUFFER_SIZE bytes of data
//
//-----------------------------------------------------------------------------
void SPI_Array_Read (void)
{
   while (!NSSMD0);                    // Wait until the SPI is free, in case
                                       // it's already busy

   NSSMD0 = 0;

   Command = SPI_READ_BUFFER;

   SPI0DAT = Command;

   // The rest of this command will be handled by the SPI ISR, which will
   // trigger when SPIF is set from sending the Command
}

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------