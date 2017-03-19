#ifndef  _SPI_H_
#define  _SPI_H_

void SPI0_Init(void);
void SPI_Byte_Write (void);
void SPI_Byte_Read (void);
void SPI_Array_Write (void);
void SPI_Array_Read (void);

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------
#define SPI_CLOCK          1000000      // Maximum SPI clock
                                        // The SPI clock is a maximum of 250 kHz
                                        // when this example is used with
                                        // the SPI0_Slave code example.

#define MAX_BUFFER_SIZE    2           // Maximum buffer Master will send

// Instruction Set
#define  SPI_WRITE         0x04        // Send a byte from the Master to the
                                       // Slave
#define  SPI_READ          0x08        // Send a byte from the Slave to the
                                       // Master
#define  SPI_WRITE_BUFFER  0x10        // Send a series of bytes from the
                                       // Master to the Slave
#define  SPI_READ_BUFFER   0x20        // Send a series of bytes from the Slave
                                       // to the Master
#define  ERROR_OCCURRED    0x40        // Indicator for the Slave to tell the

#endif