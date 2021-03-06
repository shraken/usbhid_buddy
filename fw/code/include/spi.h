/**
 * @file spi.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-09-26
 * @brief SPI driver library for configuration and data
 *				transmission using the SPI peripheral.
 *			
 */

#ifndef  _SPI_H_
#define  _SPI_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <compiler_defs.h>
#include <C8051F380_defs.h>
#include <c8051f3xx.h>
#include "globals.h"
#include "gpio.h"

extern uint8_t spi_data_rx[];
extern uint8_t spi_data_tx[];
extern uint8_t spi_bytes_trans;

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------
#define SPI_CLOCK          12000000      // Maximum SPI clock
                                        // The SPI clock is a maximum of 250 kHz
                                        // when this example is used with
                                        // the SPI0_Slave code example.

#define SPI_MAX_BUFFER_SIZE    128      // Maximum buffer Master will send

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

typedef enum _SPI_DEVICE_TYPE {
	SPI_DEVICE_TYPE_TLV563x = 0,
	SPI_DEVICE_TYPE_23LC1024,
} SPI_DEVICE_TYPE;

void spi_init(void);
void spi_array_readwrite(void);
void spi_select(void);

#endif /* _SPI_H_ */