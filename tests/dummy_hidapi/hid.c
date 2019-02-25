#include "hidapi.h"

static hidapi_dummy_context_t ctx;

/// context
void clear_context(void) {
    memset(&ctx, 0, sizeof(ctx));
}

hidapi_dummy_context_t *get_context(void) {
    return &ctx;
}

/// dummy stubs

int hid_init(void) {
    //printf("dummy hid_init invoked\n");
    ctx.fn_hid_init = true;
    return ctx.rc_hid_init;
}

hid_device *hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number) {
    //printf("dummy hid_open invoked\n");
    ctx.fn_hid_open = true;

    ctx.param_hid_init_vendor_id = vendor_id;
    ctx.param_hid_init_product_id = product_id;
    ctx.param_hid_init_serial_number = (wchar_t *) serial_number;

    return ctx.rc_hid_open;
}

int hid_get_manufacturer_string(hid_device *device, wchar_t *string, size_t maxlen) {
    //printf("dummy hid_get_manufacturer_string invoked\n");
    ctx.fn_hid_get_manufacturer_string = true;

    wcscpy(string, WIDE_HID_INIT_MANUFACTURER_PARAM);
    return ctx.rc_hid_get_manufacturer_string;
}

int hid_get_product_string(hid_device *device, wchar_t *string, size_t maxlen) {
    //printf("dummy hid_get_product_string invoked\n");
    ctx.fn_hid_get_product_string = true;

    wcscpy(string, WIDE_HID_INIT_PRODUCT_PARAM);
    return ctx.rc_hid_get_product_string;
}

int hid_get_serial_number_string(hid_device *device, wchar_t *string, size_t maxlen) {
    //printf("dummy hid_get_serial_number_string invoked\n");
    ctx.fn_hid_get_serial_number_string = true;

    wcscpy(string, WIDE_HID_INIT_SERIAL_PARAM);
    return ctx.rc_hid_get_serial_number_string;
}

int hid_get_indexed_string(hid_device *device, int string_index, wchar_t *string, size_t maxlen) {
    //printf("dummy hid_get_indexed_string invoked\n");
    ctx.fn_hid_get_indexed_string = true;

    wcscpy(string, WIDE_HID_INIT_INDEXED_PARAM);
    return ctx.rc_hid_get_indexed_string;
}

int hid_set_nonblocking(hid_device *device, int nonblock) {
    //printf("dummy hid_set_nonblocking invoked\n");
    ctx.fn_hid_set_nonblocking = true;
    ctx.nonblocking = nonblock;

    return ctx.rc_hid_set_nonblocking;
}

int hid_write(hid_device *device, const unsigned char *data, size_t length) {
    //printf("dummy hid_write invoked\n");
    ctx.fn_hid_write = true;

    if (length < MAX_HID_WRITE_BUFFER_SIZE) {
        memcpy(ctx.outbuf_hid_write, data, length);
        ctx.outbuf_copy_len_hid_write = length;
    } else {
        memcpy(ctx.outbuf_hid_write, data, MAX_HID_WRITE_BUFFER_SIZE);
        ctx.outbuf_copy_len_hid_write = MAX_HID_WRITE_BUFFER_SIZE;
    }

    return ctx.rc_hid_write;
}

int hid_read(hid_device *device, unsigned char *data, size_t length) {
    //printf("dummy hid_read invoked\n");
    ctx.fn_hid_read = true;

    if (length >= sizeof(ctx.inbuf_hid_read)) {
        memcpy(data, ctx.inbuf_hid_read, length);
    } else {
        memcpy(data, ctx.inbuf_hid_read, sizeof(ctx.inbuf_hid_read));
    }

    return ctx.rc_hid_read;
}

int hid_exit(void) {
    //printf("dummy hid_exit invoked\n");
    ctx.fn_hid_exit = true;
    return ctx.rc_hid_exit;
}

void hid_close(hid_device *device) {
    //printf("dummy hid_close invoked\n");
    ctx.fn_hid_close = true;
}

const wchar_t* hid_error(hid_device *device) {
    //printf("dummy hid_error invoked\n");
    ctx.fn_hid_error = true;
    return ctx.rc_hid_error;
}