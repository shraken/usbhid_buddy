#include <uart.h>
#include <c8051f3xx.h>
#include <globals.h>

void UART0_Init (void)
{
   SCON0 = 0x10;                       // SCON0: 8-bit variable bit rate
                                       //        level of STOP bit is ignored
                                       //        RX enabled
                                       //        ninth bits are zeros
                                       //        clear RI0 and TI0 bits

   TH1 = -(SYSCLK/BAUDRATE/2);
   CKCON &= ~0x0B;                     // T1M = 1; SCA1:0 = xx
   CKCON |=  0x08;

   TL1 = TH1;                          // Init Timer1
   TMOD &= ~0xf0;                      // TMOD: timer 1 in 8-bit autoreload
   TMOD |=  0x20;                       
   TR1 = 1;                            // START Timer1
   TI0 = 1;                            // Indicate TX0 ready
}