#include "init.h"

/** @brief This top-level initialization routine calls all support routine.
 *  @return Void.
 */
void system_init(void)
{
   PCA0MD &= ~0x40;                    // Disable Watchdog timer

   OSCICN |= 0x03;                     // Configure internal oscillator for
                                       // its maximum frequency
   RSTSRC  = 0x04;                     // Enable missing clock detector
    
   sysclk_init();                      // Initialize oscillator
   port_init();                        // Initialize crossbar and GPIO
   usb_init();                         // Initialize USB0
}

/** @brief Initialize system clock to maximum frequency.
 *  @return Void.
 */
void sysclk_init(void)
{
#ifdef _USB_LOW_SPEED_

   OSCICN |= 0x03;                     // Configure internal oscillator for
                                       // its maximum frequency and enable
                                       // missing clock detector

   CLKSEL  = SYS_INT_OSC;              // Select System clock
   CLKSEL |= USB_INT_OSC_DIV_2;        // Select USB clock
#else
   OSCICN |= 0x03;                     // Configure internal oscillator for
                                       // its maximum frequency and enable
                                       // missing clock detector

   CLKMUL  = 0x00;                     // Select internal oscillator as
                                       // input to clock multiplier

   CLKMUL |= 0x80;                     // Enable clock multiplier
   Delay();                            // Delay for clock multiplier to begin
   CLKMUL |= 0xC0;                     // Initialize the clock multiplier
   Delay();                            // Delay for clock multiplier to begin

   while(!(CLKMUL & 0x20));            // Wait for multiplier to lock
   CLKSEL  = SYS_INT_OSC;              // Select system clock
   CLKSEL |= USB_4X_CLOCK;             // Select USB clock
#endif  /* _USB_LOW_SPEED_ */
}

/** @brief Routine configure the Crossbar and GPIO ports.
 *  @return Void.
 */
void port_init(void)
{	
    P2MDIN    = 0x00;

    P0MDOUT   = 0x10;

	P1MDOUT   = 0x85;
	
	P3MDOUT   = 0x30;
	
	P4MDOUT   = 0x80;
	
    P0SKIP    = 0xCF;
	P1SKIP 	  = 0xF0;
	P2SKIP    = 0xFF;
    P3SKIP    = 0xFC;
	
    XBR0      = 0x07;
    XBR1      = 0x40;
	
		// set SPI chip select DAC and external memory to be disabled by default
		gpio_set_pin_value(SPI_DAC_CS_PIN, GPIO_VALUE_HIGH);
}

/** @brief Initialize USB0. Enable USB0 interrupts. Enable USB0 transceiver.
 *         Enable USB0 with suspend detection
 *  @return Void.
 */
void usb_init(void)
{
   POLL_WRITE_BYTE(POWER,  0x08);      // Force Asynchronous USB Reset
   POLL_WRITE_BYTE(IN1IE,  0x07);      // Enable Endpoint 0-2 in interrupts
   POLL_WRITE_BYTE(OUT1IE, 0x07);      // Enable Endpoint 0-2 out interrupts
   POLL_WRITE_BYTE(CMIE,   0x07);      // Enable Reset, Resume, and Suspend
                                       // interrupts
#ifdef _USB_LOW_SPEED_
   USB0XCN = 0xC0;                     // Enable transceiver; select low speed
   POLL_WRITE_BYTE(CLKREC, 0xA9);      // Enable clock recovery; single-step
                                       // mode disabled; low speed mode enabled
#else
   USB0XCN = 0xE0;                     // Enable transceiver; select full speed
   POLL_WRITE_BYTE(CLKREC, 0x8F);      // Enable clock recovery, single-step
                                       // mode disabled
#endif /* _USB_LOW_SPEED_ */

   EIE1 |= 0x02;                       // Enable USB0 Interrupts
   EA = 1;                             // Global Interrupt enable
                                       // Enable USB0 by clearing the USB
                                       // Inhibit bit
   POLL_WRITE_BYTE(POWER,  0x01);      // and enable suspend detection
   //Setup_IN_BUFFER();
}

/** @brief Busy wait delay routine used for USB timing.
 *  @return Void.
 */
void Delay(void)
{
   volatile int x;
	
   for(x = 0;x < 500;x)
      x++;
}