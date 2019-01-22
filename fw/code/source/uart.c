#include "uart.h"
#include "buddy_common.h"

/** @brief Sets up the UART device by configuring the crossbar for UART operation pin
 *         pin mode on pins P0.5/P0.4 with baudrate provided by BAUDRATE define.  Uses
 *				 timer1 for baud rate generation.
 *  @return Void.
 */
void uart_init (void)
{
   SCON0 = 0x10;                       // SCON0: 8-bit variable bit rate
                                       //        level of STOP bit is ignored
                                       //        RX enabled
                                       //        ninth bits are zeros
                                       //        clear RI0 and TI0 bits

   TH1 = -(BUDDY_SYSCLK/BAUDRATE/2);
   CKCON &= ~0x0B;                     // T1M = 1; SCA1:0 = xx
   CKCON |=  0x08;

   TL1 = TH1;                          // Init Timer1
   TMOD &= ~0xf0;                      // TMOD: timer 1 in 8-bit autoreload
   TMOD |=  0x20;                       
   TR1 = 1;                            // START Timer1
   TI0 = 1;                            // Indicate TX0 ready
}