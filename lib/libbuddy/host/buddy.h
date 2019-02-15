#ifndef _USBHID_BUDDY
#define _USBHID_BUDDY

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <wchar.h>
#include "buddy_common.h"
#include "hidapi.h"

//#define LABVIEW_BUILD _USRDLL

// Defined by VStudio project if a DLL is building
// to allow easy exports
#if defined(LABVIEW_BUILD)
#define BUDDY_EXPORT __declspec(dllexport)
#else
#define BUDDY_EXPORT /**< API export macro */
#endif

/// USB Vendor ID
#define BUDDY_USB_VID 0x10C4

/// USB Product ID
#define BUDDY_USB_PID 0x82CD

/// macro to convert Hz frequency to nsec time period
#define FREQUENCY_TO_NSEC(freq) ((1.0 / freq) * 1e9)
#define MAX_CHAR_LENGTH 255

/// max number of firmware info requests to be made on firmware
#define BUDDY_MAX_IO_ATTEMPTS 5

#define BUDDY_LOW_RESOLUTION_MIN 1
#define BUDDY_LOW_RESOLUTION_MAX 255
#define BUDDY_HIGH_RESOLUTION_MIN 1
#define BUDDY_HIGH_RESOLUTION_MAX 65535
#define BUDDY_SUPER_RESOLUTION_MIN 1
#define BUDDY_SUPER_RESOLUTION_MAX 4294967295

/// max number of msec to wait for operation, this is used as
/// the timeout for blocking operations
#define BUDDY_WAIT_LONGEST INT_MAX

/** @struct buddy_hid_info_t
 *  @brief Contains information about the USB device filled in
 *   by the hidapi library from USB enumeration on host. 
 */
typedef struct _buddy_hid_info_t {
	char *str_mfr;
	char *str_product;
	char *str_serial;
	char *str_index_1;
} buddy_hid_info_t;

/** @struct buddy_driver_context_t
 *  @brief Stores the configuration context for the current instance.
 *   These structure fields are set when the configuration is sent to
 *   the hardware device.
 */
typedef struct _buddy_driver_context {
	ctrl_general_t general;
	ctrl_runtime_t runtime;
	ctrl_timing_t timing;	
} buddy_driver_context;

/** @struct buddy_cfg_reg_t
 *  @brief helper structure to store the three configuration entires
 *   that must be sent to the device during initialization.
 */
typedef struct _buddy_cfg_reg_t {
	uint8_t type_indic;
	uint8_t *record_cfg;
	uint8_t record_len;	
} buddy_cfg_reg_t;

#define NUMBER_CFG_REG_ENTRIES 3

extern char *fw_info_dac_type_names[FIRMWARE_INFO_DAC_TYPE_LENGTH];

uint8_t *get_buffer(void);
buddy_driver_context *buddy_get_context(void);
int buddy_write_packet(hid_device *handle, unsigned char *buffer, int length);
int buddy_read_packet(hid_device *handle, unsigned char *buffer, int length);
int buddy_empty(hid_device *handle);
hid_device* hidapi_init(buddy_hid_info_t *hid_info);

BUDDY_EXPORT int buddy_configure(hid_device *handle, ctrl_general_t *general, ctrl_runtime_t *runtime, ctrl_timing_t *timing);
BUDDY_EXPORT hid_device* buddy_init(buddy_hid_info_t *hid_info, firmware_info_t *fw_info);
BUDDY_EXPORT int buddy_cleanup(hid_device *handle, buddy_hid_info_t *hid_info, bool device_disable);
BUDDY_EXPORT int buddy_send_pwm(hid_device *handle, general_packet_t *packet, bool streaming);
BUDDY_EXPORT int buddy_send_dac(hid_device *handle, general_packet_t *packet, bool streaming);
BUDDY_EXPORT int buddy_read_adc(hid_device *handle, general_packet_t *packet, bool streaming);
BUDDY_EXPORT int buddy_read_counter(hid_device *handle, general_packet_t *packet, bool streaming);
BUDDY_EXPORT int buddy_flush(hid_device *handle);
BUDDY_EXPORT int buddy_clear(hid_device *handle);

int buddy_get_firmware_info(hid_device *handle, firmware_info_t *fw_info);
int buddy_write_raw(hid_device *handle, uint8_t code, uint8_t indic, uint8_t *raw, uint8_t length);
int buddy_write_config(hid_device *handle, int control, uint8_t *buffer, int len);
int buddy_send_generic(hid_device *handle, general_packet_t *packet, bool streaming, uint8_t type);
int8_t buddy_get_response(hid_device *handle, uint8_t *buffer, uint8_t length);

int buddy_reset_device(hid_device *handle);
int buddy_read_generic(hid_device *handle, general_packet_t *packet, bool streaming);
int buddy_read_generic_noblock(hid_device *handle, general_packet_t *packet, bool streaming, int timeout);
				   
#endif