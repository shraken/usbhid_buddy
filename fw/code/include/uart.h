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

#include <F3xx_USB0_Register.h>
#include <F3xx_USB0_InterruptServiceRoutine.h>
#include <F3xx_USB0_Descriptor.h>
#include <F3xx_USB0_ReportHandler.h>
#include <c8051f3xx.h>
#include <stdio.h>

#define BAUDRATE           115200           // Baud rate of UART in bps

void uart_init (void);

#endif