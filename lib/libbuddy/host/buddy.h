#ifndef _USBHID_BUDDY
#define _USBHID_BUDDY

#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <wchar.h>

#include "hidapi.h"
#include "buddy_common.h"
#include "utility.h"
#include "support.h"

//#define LABVIEW_BUILD _USRDLL

// Defined by VStudio project if a DLL is building
// to allow easy exports
#if defined(LABVIEW_BUILD)
#define BUDDY_EXPORT __declspec(dllexport)
#else
#define BUDDY_EXPORT /**< API export macro */
#endif

// Defines
#define BUDDY_USB_VID 0x10C4
#define BUDDY_USB_PID 0x82CD

#define FREQUENCY_TO_NSEC(freq) ((1.0 / freq) * 1e9)
#define MAX_CHAR_LENGTH 255

#define BUDDY_MAX_IO_ATTEMPTS 5

#define BUDDY_LOW_RESOLUTION_MIN 1
#define BUDDY_LOW_RESOLUTION_MAX 255
#define BUDDY_HIGH_RESOLUTION_MIN 1
#define BUDDY_HIGH_RESOLUTION_MAX 65535
#define BUDDY_SUPER_RESOLUTION_MIN 1
#define BUDDY_SUPER_RESOLUTION_MAX 4294967295

#define BUDDY_WAIT_LONGEST INT_MAX

typedef struct _buddy_hid_info_t {
	char *str_mfr;
	char *str_product;
	char *str_serial;
	char *str_index_1;
} buddy_hid_info_t;

typedef struct _buddy_driver_context {
	ctrl_general_t general;
	ctrl_runtime_t runtime;
	ctrl_timing_t timing;	
} buddy_driver_context;

typedef struct _buddy_cfg_reg_t {
	uint8_t type_indic;
	uint8_t *record_cfg;
	uint8_t record_len;	
} buddy_cfg_reg_t;

typedef enum _BUDDY_RESPONSE_DRV_TYPE {
    BUDDY_RESPONSE_DRV_TYPE_INTERNAL = 0,
    BUDDY_RESPONSE_DRV_TYPE_STATUS,
    BUDDY_RESPONSE_DRV_TYPE_DATA
} BUDDY_RESPONSE_DRV_TYPE;

#define NUMBER_CFG_REG_ENTRIES 3

extern char *fw_info_dac_type_names[FIRMWARE_INFO_DAC_TYPE_LENGTH];

int buddy_write_packet(hid_device *handle, unsigned char *buffer, int length);
int buddy_read_packet(hid_device *handle, unsigned char *buffer, int length);
hid_device* hidapi_init(buddy_hid_info_t *hid_info);

BUDDY_EXPORT int buddy_configure(hid_device *handle, ctrl_general_t *general, ctrl_runtime_t *runtime, ctrl_timing_t *timing);
BUDDY_EXPORT hid_device* buddy_init(buddy_hid_info_t *hid_info, firmware_info_t *fw_info);
BUDDY_EXPORT int buddy_cleanup(hid_device *handle, buddy_hid_info_t *hid_info, bool device_disable);
BUDDY_EXPORT int buddy_send_pwm(hid_device *handle, general_packet_t *packet, bool streaming);
BUDDY_EXPORT int buddy_send_dac(hid_device *handle, general_packet_t *packet, bool streaming);
BUDDY_EXPORT int buddy_read_adc(hid_device *handle, general_packet_t *packet, bool streaming);
BUDDY_EXPORT int buddy_read_adc_noblock(hid_device *handle, general_packet_t *packet, bool streaming, int timeout);
BUDDY_EXPORT int buddy_read_counter(hid_device *handle, general_packet_t *packet, bool streaming);
BUDDY_EXPORT int buddy_flush(hid_device *handle);
BUDDY_EXPORT int buddy_clear(hid_device *handle);

int buddy_empty(hid_device *handle);

int8_t buddy_get_response(hid_device *handle, uint8_t *res_type, uint8_t *buffer, uint8_t length);

int buddy_get_firmware_info(hid_device *handle, firmware_info_t *fw_info);
int buddy_write_raw(hid_device *handle, uint8_t code, uint8_t indic, uint8_t *raw, uint8_t length);
int buddy_send_generic(hid_device *handle, general_packet_t *packet, bool streaming, uint8_t type);

int buddy_reset_device(hid_device *handle);
int buddy_read_generic(hid_device *handle, general_packet_t *packet, bool streaming);
int buddy_read_generic_noblock(hid_device *handle, general_packet_t *packet, bool streaming, int timeout);

					   
#endif