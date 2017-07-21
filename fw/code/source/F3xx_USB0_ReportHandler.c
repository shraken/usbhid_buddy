#include <stdio.h>
#include <string.h>
#include <gpio.h>
#include <globals.h>
#include <F3xx_USB0_ReportHandler.h>
#include <F3xx_USB0_InterruptServiceRoutine.h>

extern uint8_t processed_out;

// ----------------------------------------------------------------------------
// Local Function Prototypes
// ----------------------------------------------------------------------------

// ****************************************************************************
// Add custom Report Handler Prototypes Here
// ****************************************************************************

void IN_Report(void);
void OUT_Report(void);

void IN_DATA_ROUTINE(void);
void OUT_DATA_ROUTINE(void);

// ****************************************************************************
// Set Definitions to sizes corresponding to the number of reports
// ****************************************************************************
#define IN_VECTORTABLESize 1
#define OUT_VECTORTABLESize 1

// ----------------------------------------------------------------------------
// Local Definitions
// ----------------------------------------------------------------------------

extern unsigned char flag_usb_out;
extern unsigned char flag_usb_in;

unsigned char xdata OUT_PACKET[64];
unsigned char xdata IN_PACKET[64];

// ----------------------------------------------------------------------------
// Global Constant Declaration
// ----------------------------------------------------------------------------

// ****************************************************************************
// Link all Report Handler functions to corresponding Report IDs
// ****************************************************************************
const VectorTableEntry IN_VECTORTABLE[IN_VECTORTABLESize] =
{
   // FORMAT: Report ID, Report Handler
   IN_DATA, IN_DATA_ROUTINE,
};

// ****************************************************************************
// Link all Report Handler functions to corresponding Report IDs
// ****************************************************************************
const VectorTableEntry OUT_VECTORTABLE[OUT_VECTORTABLESize] =
{
   // FORMAT: Report ID, Report Handler
   OUT_DATA, OUT_DATA_ROUTINE,
};


// ----------------------------------------------------------------------------
// Global Variable Declaration
// ----------------------------------------------------------------------------

BufferStructure IN_BUFFER, OUT_BUFFER;

// ----------------------------------------------------------------------------
// Local Variable Declaration
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Local Functions
// ----------------------------------------------------------------------------

// ****************************************************************************
// Add all Report Handler routines here
// ****************************************************************************


// ****************************************************************************
// For Input Reports:
// Point IN_BUFFER.Ptr to the buffer containing the report
// Set IN_BUFFER.Length to the number of bytes that will be
// transmitted.
//
// REMINDER:
// Systems using more than one report must define Report IDs
// for each report.  These Report IDs must be included as the first
// byte of an IN report.
// Systems with Report IDs should set the FIRST byte of their buffer
// to the value for the Report ID
// AND
// must transmit Report Size + 1 to include the full report PLUS
// the Report ID.
//
// ****************************************************************************


// ----------------------------------------------------------------------------
// IN_DATA_ROUTINE()
// ----------------------------------------------------------------------------
// Set the USB IN (Device -> Host) buffer pointer to IN_PACKET where ADC
// samples will be copied into
void IN_DATA_ROUTINE(void){
   IN_PACKET[0] = IN_DATA;
	
   IN_BUFFER.Ptr = IN_PACKET;
   IN_BUFFER.Length = IN_DATA_SIZE + 1;
}

// ****************************************************************************
// For Output Reports:
// Data contained in the buffer OUT_BUFFER.Ptr will not be
// preserved after the Report Handler exits.
// Any data that needs to be preserved should be copyed from
// the OUT_BUFFER.Ptr and into other user-defined memory.
//
// ****************************************************************************

// ----------------------------------------------------------------------------
// OUT_DATA_ROUTINE()
// ----------------------------------------------------------------------------
// Set a flag to indicate that USB OUT (Host -> Device) packet has been received
// and that DAC message should be sent
void OUT_DATA_ROUTINE(void)
{
   flag_usb_out = 1;
   processed_out = 0;
   //printf("disable USBHID out\r\n");
}

// ----------------------------------------------------------------------------
// Global Functions
// ----------------------------------------------------------------------------

// ****************************************************************************
// Configure Setup_OUT_BUFFER
//
// Reminder:
// This function should set OUT_BUFFER.Ptr so that it
// points to an array in data space big enough to store
// any output report.
// It should also set OUT_BUFFER.Length to the size of
// this buffer.
//
// ****************************************************************************

void Setup_OUT_BUFFER(void)
{
   OUT_BUFFER.Ptr = (unsigned char *) OUT_PACKET;
   OUT_BUFFER.Length = OUT_DATA_SIZE;
}

/*
void Setup_IN_BUFFER(void)
{
   // Set USB IN report buffer pointer to IN_PACKET buffer
   IN_BUFFER.Ptr = (unsigned char *) IN_PACKET;
   IN_BUFFER.Length = (IN_DATA_SIZE + 1); 
}
*/

// ----------------------------------------------------------------------------
// Vector Routines
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// ReportHandler_IN...
// ----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - Report ID
//
// These functions match the Report ID passed as a parameter
// to an Input Report Handler.
// the ...FG function is called in the SendPacket foreground routine,
// while the ...ISR function is called inside the USB ISR.  A lock
// is set whenever one function is called to prevent a call from the
// other from disrupting the routine.
// However, this should never occur, as interrupts are disabled by SendPacket
// before USB operation begins.
// ----------------------------------------------------------------------------
void ReportHandler_IN_ISR(unsigned char R_ID)
{
   unsigned char index;

   index = 0;

   while(index <= IN_VECTORTABLESize)
   {
      // Check to see if Report ID passed into function
     // matches the Report ID for this entry in the Vector Table
      if(IN_VECTORTABLE[index].ReportID == R_ID)
      {
         IN_VECTORTABLE[index].hdlr();
         break;
      }

      // If Report IDs didn't match, increment the index pointer
      index++;
   }

}

void ReportHandler_IN_Foreground(unsigned char R_ID)
{
   unsigned char index;

   index = 0;

   while(index <= IN_VECTORTABLESize)
   {
      // Check to see if Report ID passed into function
      // matches the Report ID for this entry in the Vector Table
      if(IN_VECTORTABLE[index].ReportID == R_ID)
      {
         IN_VECTORTABLE[index].hdlr();
         break;
      }

      // If Report IDs didn't match, increment the index pointer
      index++;
   }

}

// ----------------------------------------------------------------------------
// ReportHandler_OUT
// ----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// This function matches the Report ID passed as a parameter
// to an Output Report Handler.
//
// ----------------------------------------------------------------------------
void ReportHandler_OUT(unsigned char R_ID){

   unsigned char index;

   index = 0;

   while(index <= OUT_VECTORTABLESize)
   {
      // Check to see if Report ID passed into function
      // matches the Report ID for this entry in the Vector Table
      if(OUT_VECTORTABLE[index].ReportID == R_ID)
      {
         OUT_VECTORTABLE[index].hdlr();
         break;
      }

      // If Report IDs didn't match, increment the index pointer
      index++;
   }
}