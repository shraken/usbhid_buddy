#ifndef DUMMY_HIDAPI_H__
#define DUMMY_HIDAPI_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>
#include <string.h>

#define WIDE_HID_INIT_MANUFACTURER_PARAM L"manufacturer_test"
#define HID_INIT_MANUFACTURER_PARAM "manufacturer_test"

#define WIDE_HID_INIT_PRODUCT_PARAM L"product_test"
#define HID_INIT_PRODUCT_PARAM "product_test"

#define WIDE_HID_INIT_SERIAL_PARAM L"serial_test"
#define HID_INIT_SERIAL_PARAM "serial_test"

#define WIDE_HID_INIT_INDEXED_PARAM L"indexed_test"
#define HID_INIT_INDEXED_PARAM "indexed_test"

#define MAX_HID_WRITE_BUFFER_SIZE 63
#define MAX_HID_READ_BUFFER_SIZE 63

typedef struct _hid_device {
	int device_handle;
	int blocking;
	int uses_numbered_reports;
} hid_device;

typedef struct _hidapi_dummy_context_t {
	// set to true when invoked by the libbuddy host functions
	bool fn_hid_init;
	bool fn_hid_open;
	bool fn_hid_get_manufacturer_string;
	bool fn_hid_get_product_string;
	bool fn_hid_get_serial_number_string;
	bool fn_hid_get_indexed_string;
	bool fn_hid_set_nonblocking;
	bool fn_hid_write;
	bool fn_hid_read;
	bool fn_hid_exit;
	bool fn_hid_close;
	bool fn_hid_error;

	// return code when the described method is called
	int rc_hid_init;
	hid_device* rc_hid_open;
	int rc_hid_get_manufacturer_string;
	int rc_hid_get_product_string;
	int rc_hid_get_serial_number_string;
	int rc_hid_get_indexed_string;
	int rc_hid_set_nonblocking;
	int rc_hid_write;
	int rc_hid_read;
	int rc_hid_exit;
	wchar_t* rc_hid_error;

	// parameter storage
	unsigned short param_hid_init_vendor_id;
	unsigned short param_hid_init_product_id;
	wchar_t *param_hid_init_serial_number;
	
	int nonblocking;
	uint8_t outbuf_hid_write[MAX_HID_WRITE_BUFFER_SIZE];
	int outbuf_copy_len_hid_write;

	uint8_t inbuf_hid_read[MAX_HID_READ_BUFFER_SIZE];
} hidapi_dummy_context_t;

void clear_context(void);
hidapi_dummy_context_t *get_context(void);

int hid_init(void);
hid_device *hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number);
int hid_get_manufacturer_string(hid_device *device, wchar_t *string, size_t maxlen);
int hid_get_product_string(hid_device *device, wchar_t *string, size_t maxlen);
int hid_get_serial_number_string(hid_device *device, wchar_t *string, size_t maxlen);
int hid_get_indexed_string(hid_device *device, int string_index, wchar_t *string, size_t maxlen);
int hid_set_nonblocking(hid_device *device, int nonblock);
int hid_write(hid_device *device, const unsigned char *data, size_t length);
int hid_read(hid_device *device, unsigned char *data, size_t length);
int hid_exit(void);
void hid_close(hid_device *device);
const wchar_t* hid_error(hid_device *device);

#endif