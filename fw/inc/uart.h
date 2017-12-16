/**
 * @file uart.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-09-26
 * @brief Initialization library for the UART0 peripheral. 
 *			
 */

#ifndef _UART_H
#define _UART_H

#include <compiler_defs.h>
#include <c8051f380.h>
#include <globals.h>

#define BAUDRATE           115200           // Baud rate of UART in bps
//#define BAUDRATE           9600

/** @brief Sets up the UART device by configuring the crossbar for UART operation pin
 *         pin mode on pins P0.5/P0.4 with baudrate provided by BAUDRATE define.  Uses
 *				 timer1 for baud rate generation.
 *  @return Void.
 */
void uart_init(void);

#endif