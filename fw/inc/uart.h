/** @file uart.h
 *  @brief Function prototypes for the usart routines.
 *
 */
 
#ifndef _C8051F380_USART_H
#define _C8051F380_USART_H

#include <compiler_defs.h>
#include <c8051f380.h>
#include <globals.h>

#define BAUDRATE      115200           // Baud rate of UART in bps

void UART0_Init (void);

#endif