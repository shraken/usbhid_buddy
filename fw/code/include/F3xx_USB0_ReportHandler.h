//-----------------------------------------------------------------------------
// F3xx_USB0_ReportHandler.h
//-----------------------------------------------------------------------------
// Copyright 2010 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// Includes functions called by USB_ISR.c to handle input and output reports.//
//
// How To Test:    See Readme.txt
//
//
// FID             3XX000014
// Target:         C8051F32x/C8051F340
// Tool chain:     Keil / Raisonance
//                 Silicon Laboratories IDE version 2.6
// Command Line:   See Readme.txt
// Project Name:   F3xx_BlinkyExample
//
// Release 1.2 (ES)
//    -Added support for Raisonance
//    -No change to this file
//    -02 APR 2010
// Release 1.1
//    -Added feature reports for dimming controls
//    -Added PCA dimmer functionality
//    -16 NOV 2006
// Release 1.0
//    -Initial Revision (PD)
//    -07 DEC 2005
//

#ifndef  _USB_REPORTHANDLER_H_
#define  _USB_REPORTHANDLER_H_

#define OUT_DATA 0x01
#define IN_DATA 0x02

#define OUT_DATA_SIZE 63
#define IN_DATA_SIZE 63

typedef struct {
   unsigned char ReportID;
   void (*hdlr)();
} VectorTableEntry;

typedef struct{
   unsigned char Length;
   unsigned char* Ptr;
} BufferStructure;

extern void ReportHandler_IN_ISR(unsigned char);
extern void ReportHandler_IN_Foreground(unsigned char);
extern void ReportHandler_OUT(unsigned char);
extern void Setup_OUT_BUFFER(void);

extern BufferStructure IN_BUFFER;
extern BufferStructure OUT_BUFFER;

void IN_DATA_ROUTINE(void);
void OUT_DATA_ROUTINE(void);
void Setup_OUT_BUFFER(void);
void ReportHandler_IN_ISR(unsigned char R_ID);
void ReportHandler_IN_Foreground(unsigned char R_ID);
void ReportHandler_OUT(unsigned char R_ID);

#endif