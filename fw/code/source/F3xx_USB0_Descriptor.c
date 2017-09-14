#include <F3xx_USB0_Register.h>
#include <F3xx_USB0_InterruptServiceRoutine.h>
#include <F3xx_USB0_Descriptor.h>
#include <F3xx_USB0_ReportHandler.h>

extern unsigned char xdata OUT_PACKET[];
extern unsigned char xdata IN_PACKET[];

//-----------------------------------------------------------------------------
// Descriptor Declarations
//-----------------------------------------------------------------------------

code const device_descriptor DEVICEDESC =
{
   18,                                 // bLength
   0x01,                               // bDescriptorType
   0x1001,                             // bcdUSB
   0x00,                               // bDeviceClass
   0x00,                               // bDeviceSubClass
   0x00,                               // bDeviceProtocol
   EP0_PACKET_SIZE,                    // bMaxPacketSize0
   0xC410,                             // idVendor
   0x0240,                             // idProduct
   0x0000,                             // bcdDevice
   0x01,                               // iManufacturer
   0x02,                               // iProduct
   0x00,                               // iSerialNumber
   0x01                                // bNumConfigurations
}; //end of DEVICEDESC

// From "USB Device Class Definition for Human Interface Devices (HID)".
// Section 7.1:
// "When a Get_Descriptor(Configuration) request is issued,
// it returns the Configuration descriptor, all Interface descriptors,
// all Endpoint descriptors, and the HID descriptor for each interface."
code const buddy_configuration_descriptor BUDDYCONFIGDESC =
{

{ // configuration_descriptor hid_configuration_descriptor
   0x09,                               // Length
   0x02,                               // Type
   //0x2900,                             // Totallength (= 9+9+9+7+7)
   0x4000,														 // Totallength (= 9+9+9+7+7 + 9+7+7)
	 0x02,                               // NumInterfaces
   0x01,                               // bConfigurationValue
   0x00,                               // iConfiguration
   0x80,                               // bmAttributes
   0x20                                // MaxPower (in 2mA units)
},

{ // interface_descriptor hid_interface_descriptor
   0x09,                               // bLength
   0x04,                               // bDescriptorType
   0x00,                               // bInterfaceNumber
   0x00,                               // bAlternateSetting
   0x02,                               // bNumEndpoints
   0x03,                               // bInterfaceClass (3 = HID)
   0x00,                               // bInterfaceSubClass
   0x00,                               // bInterfaceProcotol
   0x00                                // iInterface
},

{ // class_descriptor hid_descriptor
   0x09,                               // bLength
   0x21,                               // bDescriptorType
   0x0101,                             // bcdHID
   0x00,                               // bCountryCode
   0x02,                               // bNumDescriptors
   0x22,                               // bDescriptorType
   HID_REPORT_DESCRIPTOR_SIZE_LE       // wItemLength (tot. len. of report
                                       // descriptor)
},

// IN endpoint (mandatory for HID)
{ // endpoint_descriptor hid_endpoint_in_descriptor
   0x07,                               // bLength
   0x05,                               // bDescriptorType
   0x81,                               // bEndpointAddress
   0x03,                               // bmAttributes
   EP1_PACKET_SIZE_LE,                 // MaxPacketSize (LITTLE ENDIAN)
   1                                   // bInterval
},

// OUT endpoint (optional for HID)
{ // endpoint_descriptor hid_endpoint_out_descriptor
   0x07,                               // bLength
   0x05,                               // bDescriptorType
   0x01,                               // bEndpointAddress
   0x03,                               // bmAttributes
   EP1_PACKET_SIZE_LE,                 // MaxPacketSize (LITTLE ENDIAN)
   1                                   // bInterval
},

{ // interface_descriptor bulk_interface_descriptor
   0x09,                               // bLength
   0x04,                               // bDescriptorType
   0x01,                               // bInterfaceNumber
   0x00,                               // bAlternateSetting
   0x02,                               // bNumEndpoints
   0xFF,                               // bInterfaceClass (Vendor specific)
   0x00,                               // bInterfaceSubClass
   0x00,                               // bInterfaceProcotol
   0x00                                // iInterface
},

// IN bulk endpoint
{ // endpoint_descriptor bulk_endpoint_in_descriptor
   0x07,                               // bLength
   0x05,                               // bDescriptorType
   0x82,                               // bEndpointAddress
   0x02,                               // bmAttributes
   EP2_PACKET_SIZE_LE,                 // MaxPacketSize (LITTLE ENDIAN)
   1                                   // bInterval
},

// OUT bulk endpoint
{ // endpoint_descriptor bulk_endpoint_out_descriptor
   0x07,                               // bLength
   0x05,                               // bDescriptorType
   0x02,                               // bEndpointAddress
   0x02,                               // bmAttributes
   EP2_PACKET_SIZE_LE,                 // MaxPacketSize (LITTLE ENDIAN)
   1                                   // bInterval
},

};

code const hid_report_descriptor HIDREPORTDESC =
{
    0x06, 0x00, 0xff,              // USAGE_PAGE (Vendor Defined Page 1)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)

    0x85, OUT_DATA,                // Report ID
    0x95, OUT_DATA_SIZE,           //   REPORT_COUNT ()
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    
    0x85, IN_DATA,                 // Report ID
    0x95, IN_DATA_SIZE,            //   REPORT_COUNT ()
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    
    0xC0                           //   end Application Collection
};

#define STR0LEN 4

code unsigned char String0Desc [STR0LEN] =
{
   STR0LEN, 0x03, 0x09, 0x04
}; // End of String0Desc

#define STR1LEN sizeof ("WIGGLE") * 2

code unsigned char String1Desc [STR1LEN] =
{
   STR1LEN, 0x03,
   'W', 0,
   'I', 0,
   'G', 0,
   'G', 0,
   'L', 0,
   'E', 0
}; // End of String1Desc

#define STR2LEN sizeof ("BUDDY") * 2

code unsigned char String2Desc [STR2LEN] =
{
   STR2LEN, 0x03,
   'B', 0,
   'U', 0,
   'D', 0,
   'D', 0,
   'Y', 0
}; // End of String2Desc

unsigned char* xdata STRINGDESCTABLE [] =
{
   String0Desc,
   String1Desc,
   String2Desc
};