#ifndef _INIT_H
#define _INIT_H

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------
// USB clock selections (SFR CLKSEL)
#define USB_4X_CLOCK       0x00        // Select 4x clock multiplier, for USB
#define USB_INT_OSC_DIV_2  0x10        // Full Speed
#define USB_EXT_OSC        0x20
#define USB_EXT_OSC_DIV_2  0x30
#define USB_EXT_OSC_DIV_3  0x40
#define USB_EXT_OSC_DIV_4  0x50

// System clock selections (SFR CLKSEL)
//#define SYS_INT_OSC        0x00        // Select to use internal oscillator
#define SYS_INT_OSC        0x03

#define SYS_EXT_OSC        0x01        // Select to use an external oscillator
#define SYS_4X_DIV_2       0x02

#define ON  1
#define OFF 0

void System_Init(void);
void Sysclk_Init(void);
void Port_Init(void);
void Usb_Init(void);
void Delay(void);
void DelayLong(void);

#endif