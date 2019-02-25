#define CATCH_CONFIG_MAIN

#include "catch.hpp"

extern "C" {
#include "buddy_common.h"
#include "codec.h"
#include "support.h"
#include "utility.h"
#include "hidapi.h"
#include "buddy.h"
}

#include <vector>
#include <tuple>
#include <cwchar>

#include <stdio.h>
#include <sys/timeb.h> 

extern uint8_t _data_size;

void clear_channels(general_packet_t &packet) {
    for (int i = BUDDY_CHAN_0; i < BUDDY_CHAN_7; i++) {
        packet.channels[i] = 0;
        codec_set_channel_active(i, false);
    }
}

void cleanup_hid_device(buddy_hid_info_t &info) {
    free(info.str_index_1);
    free(info.str_mfr);
    free(info.str_product);
    free(info.str_serial);
}

void cleanup_hid_device(buddy_hid_info_t &info, hid_device *p) {
    cleanup_hid_device(info);
    free(p);
}

TEST_CASE( "endian swap routines", "" ) {
    REQUIRE( swap_uint16(0x1234) == 0x3412 );
    REQUIRE( swap_int16(0x1234) == 0x3412 );

    REQUIRE( swap_uint32(0x12345678) == 0x78563412 );
    REQUIRE( swap_int32(0x12345678) == 0x78563412 );
}

TEST_CASE( "time sleep functions keep time and delay properly =", "" ) {
    struct timeb start, end;
    const int DEFAULT_WAIT_TIME_MSEC = 100; // in msec
    
    ftime(&start);
    short_sleep(DEFAULT_WAIT_TIME_MSEC);
    ftime(&end);

    int diff = (int) (1000.0 * (end.time - start.time) + (end.millitm - start.millitm));
    REQUIRE( diff >= DEFAULT_WAIT_TIME_MSEC );
}

TEST_CASE( "buddy codec has it's state and offsets reset", "" ) {
    codec_reset();

    REQUIRE( codec_get_offset_count() == 0 );
    REQUIRE( codec_get_encode_count() == 0 );
    REQUIRE( codec_get_decode_count() == 0 );
}

TEST_CASE( "codec allows data size and resolution to be set", "" ) {
    codec_set_data_size(BUDDY_DATA_SIZE_LOW);
    REQUIRE( codec_get_data_size() == BUDDY_DATA_SIZE_LOW );

    codec_set_data_size(BUDDY_DATA_SIZE_HIGH);
    REQUIRE( codec_get_data_size() == BUDDY_DATA_SIZE_HIGH );

    codec_set_data_size(RESOLUTION_CTRL_SUPER);
    REQUIRE( codec_get_data_size() == RESOLUTION_CTRL_SUPER );

    REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );
    REQUIRE( codec_get_resolution() == RESOLUTION_CTRL_LOW );

    REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_HIGH) == 0 );
    REQUIRE( codec_get_resolution() == RESOLUTION_CTRL_HIGH );

    REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_SUPER) == 0 );
    REQUIRE( codec_get_resolution() == RESOLUTION_CTRL_SUPER );
}

TEST_CASE( "codec channel count is correct when the codec_init initialization function is called", "" ) {
    REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );
    REQUIRE( codec_get_channel_count() == 1 );

    REQUIRE( codec_init(BUDDY_CHAN_0_MASK | BUDDY_CHAN_1_MASK, RESOLUTION_CTRL_LOW) == 0 );
    REQUIRE( codec_get_channel_count() == 2 );

    REQUIRE( codec_init(BUDDY_CHAN_0_MASK | BUDDY_CHAN_1_MASK | BUDDY_CHAN_2_MASK, RESOLUTION_CTRL_LOW) == 0 );
    REQUIRE( codec_get_channel_count() == 3 );

    REQUIRE( codec_init(BUDDY_CHAN_0_MASK | BUDDY_CHAN_1_MASK | BUDDY_CHAN_2_MASK | BUDDY_CHAN_3_MASK |
                        BUDDY_CHAN_4_MASK | BUDDY_CHAN_5_MASK | BUDDY_CHAN_6_MASK | BUDDY_CHAN_7_MASK,
                        RESOLUTION_CTRL_LOW) == 0 );
    REQUIRE( codec_get_channel_count() == 8 );
}

TEST_CASE( "codec should allow encode and decode counts to be set and the counts should match when retrieved", "" ) {
    const int MAGIC_DECODE_COUNT_VALUE = 20;
    const int MAGIC_ENCODE_COUNT_VALUE = 30;

    codec_set_decode_count(MAGIC_DECODE_COUNT_VALUE);
    REQUIRE( codec_get_decode_count() == MAGIC_DECODE_COUNT_VALUE );

    codec_set_encode_count(MAGIC_ENCODE_COUNT_VALUE);
    REQUIRE( codec_get_encode_count() == MAGIC_ENCODE_COUNT_VALUE );
}

TEST_CASE( "buddy frame encoder packages packets correctly", "" ) {
    uint8_t test_buffer[MAX_REPORT_SIZE];
    general_packet_t packet;

    REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );

    SECTION( "buddy encode rejects invalid parameters", "" ) {
        REQUIRE( codec_encode(nullptr, (general_packet_t *) &packet) == CODEC_STATUS_ERROR );
        REQUIRE( codec_encode( (uint8_t *) &test_buffer, nullptr) == CODEC_STATUS_ERROR );

        const int NON_EXIST_RESOLUTION = 100;
        codec_set_resolution(NON_EXIST_RESOLUTION);
        REQUIRE( codec_encode( (uint8_t *) &test_buffer, &packet) == CODEC_STATUS_ERROR );
    }

    SECTION( "codec encode operation should report failure code if codec is not initialized") {
        codec_reset();
        REQUIRE( codec_encode( (uint8_t *) &test_buffer, &packet ) == CODEC_STATUS_UNINITIALIZED );
    }

    SECTION( "buddy encode correctly downscales packet values for lower resolutions", "" ) {
        clear_channels(packet);

        // expected response from the encode function.  the encode method
        // should downscale the resolution by bit shift when a larger
        // resolution is being packed in a lower resolution
        //
        // resolution_mode, data_size, packet value, encoded value
        std::vector<std::tuple<int, int, uint32_t, uint32_t>> expectedResponse = {
            std::make_tuple(RESOLUTION_CTRL_LOW, BUDDY_DATA_SIZE_LOW, 0xFFFFFF, 0xFF),
            std::make_tuple(RESOLUTION_CTRL_LOW, BUDDY_DATA_SIZE_LOW, 0xFFFF, 0xFF),
            std::make_tuple(RESOLUTION_CTRL_LOW, BUDDY_DATA_SIZE_LOW, 0xFF, 0xFF),
            std::make_tuple(RESOLUTION_CTRL_LOW, BUDDY_DATA_SIZE_LOW, 0x7F, 0x7F),
            std::make_tuple(RESOLUTION_CTRL_LOW, BUDDY_DATA_SIZE_LOW, 0x00, 0x00),

            std::make_tuple(RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH, 0xFFFFFFFF, 0xFFFF),
            std::make_tuple(RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH, 0xFFFF, 0xFFFF),
            std::make_tuple(RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH, 0x7FFF, swap_uint16(0x7FFF)),
            std::make_tuple(RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH, 0x0000, 0x0000),

            std::make_tuple(RESOLUTION_CTRL_SUPER, BUDDY_DATA_SIZE_SUPER, 0xFFFFFFFF, 0xFFFFFFFF),
            std::make_tuple(RESOLUTION_CTRL_SUPER, BUDDY_DATA_SIZE_SUPER, 0x7FFFFFFF, swap_uint32(0x7FFFFFFF)),
            std::make_tuple(RESOLUTION_CTRL_SUPER, BUDDY_DATA_SIZE_SUPER, 0x00, 0x00),
        };

        for (const auto &item : expectedResponse) {
            int res_mode = std::get<0>(item);
            int data_size = std::get<1>(item);
            uint32_t in_value = std::get<2>(item);
            uint32_t out_value_expect = std::get<3>(item);

            REQUIRE( codec_init(BUDDY_CHAN_0_MASK, res_mode) == 0 );

            packet.channels[BUDDY_CHAN_0] = in_value;
            codec_set_channel_active(BUDDY_CHAN_0, true);

            int old_codec_byte_offset = codec_get_offset_count();
            REQUIRE( codec_encode((uint8_t *) &test_buffer, &packet) == CODEC_STATUS_CONTINUE );
            REQUIRE( codec_get_offset_count() == (old_codec_byte_offset + data_size) );
            
            uint32_t frame_channel_value = 0;

            memcpy(&frame_channel_value, 
                (uint8_t *) (test_buffer + BUDDY_APP_VALUE_OFFSET), data_size);
            REQUIRE( frame_channel_value == out_value_expect );
        }
    }

    SECTION( "buddy encode correctly reaches full status for each resolution mode", "" ) {
        clear_channels(packet);

        // number of packets we can fit in frame
        const int MAX_EXPECTED_PACKET_FILLS = MAX_REPORT_SIZE - BUDDY_APP_VALUE_OFFSET - 1;

        // <RESOLUTION_CTRL enum>, <BUDDY_DATA_SIZE>, <expected_count>
        // expected_count is the number of times we invoke the `encode` method before
        // expecting that that the buffer has been filled.
        // 
        std::vector<std::tuple<int, int, int>> expectedResponse = {
            std::make_tuple(RESOLUTION_CTRL_LOW,  BUDDY_DATA_SIZE_LOW, MAX_EXPECTED_PACKET_FILLS),
            std::make_tuple(RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH, MAX_EXPECTED_PACKET_FILLS / 2),
            std::make_tuple(RESOLUTION_CTRL_SUPER, BUDDY_DATA_SIZE_SUPER, MAX_EXPECTED_PACKET_FILLS / 4),
        };

        // only activate channel 0
        codec_set_channel_active(BUDDY_CHAN_0, true);

        for (const auto &item : expectedResponse) {
            int res_ctrl = std::get<0>(item);
            int buddy_size = std::get<1>(item);
            int expectedCount = std::get<2>(item);

            REQUIRE( codec_init(BUDDY_CHAN_0_MASK, res_ctrl) == 0 );

            // initials, only do one less iteration so that the last operation gets the full return code
            for (int i = 0; i < (expectedCount - 1); i++) {
                REQUIRE( codec_encode((uint8_t *) &test_buffer, &packet) == CODEC_STATUS_CONTINUE );

                if (res_ctrl == RESOLUTION_CTRL_LOW) {
                    REQUIRE( codec_get_offset_count() == (i + 1) );
                } else if (res_ctrl == RESOLUTION_CTRL_HIGH) {
                    REQUIRE( codec_get_offset_count() == ((i + 1) * 2) );
                } else if (res_ctrl == RESOLUTION_CTRL_SUPER) {
                    REQUIRE( codec_get_offset_count() == ((i + 1) * 4) );
                }
            }

            // final
            REQUIRE( codec_encode((uint8_t *) &test_buffer, &packet) == CODEC_STATUS_FULL );
            REQUIRE( codec_get_offset_count() == 0 );
        }
    }

    SECTION( "buddy codec init correctly rejects invalid resolution modes", "" ) {
        REQUIRE( codec_init(0, RESOLUTION_CTRL_END_MARKER) == -1 );
    }
}

TEST_CASE( "buddy frame decoder parses frame to packets correctly", "" ) {
    uint8_t test_buffer[MAX_REPORT_SIZE];
    general_packet_t packet;

    REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );

    SECTION( "buddy decode rejects invalid parameters", "" ) {
        REQUIRE( codec_decode(nullptr, (general_packet_t *) &packet) == CODEC_STATUS_ERROR );
        REQUIRE( codec_decode( (uint8_t *) &test_buffer, nullptr) == CODEC_STATUS_ERROR );

        const int NON_EXIST_RESOLUTION = 100;
        codec_set_resolution(NON_EXIST_RESOLUTION);
        REQUIRE( codec_decode( (uint8_t *) &test_buffer, &packet) == CODEC_STATUS_ERROR );
    }

    SECTION( "codec decode operation should report failure code if codec is not initialized") {
        codec_reset();
        REQUIRE( codec_decode( (uint8_t *) &test_buffer, &packet ) == CODEC_STATUS_UNINITIALIZED );
    }

    SECTION( "buddy decode correctly converts for the various resolutions", "" ) {
        clear_channels(packet);

        codec_set_channel_active(BUDDY_CHAN_0, true);
        codec_set_channel_active(BUDDY_CHAN_4, true);

        const uint32_t MAGIC_BUDDY_DECODE_SUPER_VALUE_0 = 0xFFFFFFFF;
        const uint16_t MAGIC_BUDDY_DECODE_HIGH_VALUE_0 = 0xFFFF;
        const uint8_t MAGIC_BUDDY_DECODE_LOW_VALUE_0 = 0xFF;

        const uint32_t MAGIC_BUDDY_DECODE_SUPER_VALUE_1 = 0x12345678;
        const uint16_t MAGIC_BUDDY_DECODE_HIGH_VALUE_1 = 0xBEEF;
        const uint8_t MAGIC_BUDDY_DECODE_LOW_VALUE_1 = 0x40;

        const uint8_t buddy_packed_low_cont[] = { 
            BUDDY_OUT_DATA_ID,
            APP_CODE_DAC,
            0x02,
            (uint8_t) MAGIC_BUDDY_DECODE_LOW_VALUE_0,
            (uint8_t) MAGIC_BUDDY_DECODE_LOW_VALUE_1,
        };

        const uint8_t buddy_packed_high_cont[] = {
            BUDDY_OUT_DATA_ID,
            APP_CODE_DAC,
            0x02,
            (uint8_t) ((MAGIC_BUDDY_DECODE_HIGH_VALUE_0 & 0xFF00) >> 8),
            (uint8_t) (MAGIC_BUDDY_DECODE_HIGH_VALUE_0 & 0xFF),

            (uint8_t) ((MAGIC_BUDDY_DECODE_HIGH_VALUE_1 & 0xFF00) >> 8),
            (uint8_t) (MAGIC_BUDDY_DECODE_HIGH_VALUE_1 & 0xFF)
        };

        const uint8_t buddy_packed_super_cont[] = {
            BUDDY_OUT_DATA_ID,
            APP_CODE_DAC,
            0x02,
            (uint8_t) ((MAGIC_BUDDY_DECODE_SUPER_VALUE_0 & 0xFF000000) >> 24),
            (uint8_t) ((MAGIC_BUDDY_DECODE_SUPER_VALUE_0 & 0x00FF0000) >> 16),
            (uint8_t) ((MAGIC_BUDDY_DECODE_SUPER_VALUE_0 & 0x0000FF00) >> 8),
            (uint8_t) (MAGIC_BUDDY_DECODE_SUPER_VALUE_0 & 0xFF),
            
            (uint8_t) ((MAGIC_BUDDY_DECODE_SUPER_VALUE_1 & 0xFF000000) >> 24),
            (uint8_t) ((MAGIC_BUDDY_DECODE_SUPER_VALUE_1 & 0x00FF0000) >> 16),
            (uint8_t) ((MAGIC_BUDDY_DECODE_SUPER_VALUE_1 & 0x0000FF00) >> 8),
            (uint8_t) (MAGIC_BUDDY_DECODE_SUPER_VALUE_1 & 0xFF),
        };

        REQUIRE( codec_init( BUDDY_CHAN_0_MASK | BUDDY_CHAN_4_MASK, RESOLUTION_CTRL_LOW) == 0 );
        REQUIRE( codec_decode( (uint8_t *) &buddy_packed_low_cont, &packet) == CODEC_STATUS_CONTINUE );
        REQUIRE( packet.channels[BUDDY_CHAN_0] == MAGIC_BUDDY_DECODE_LOW_VALUE_0 );
        REQUIRE( packet.channels[BUDDY_CHAN_4] == MAGIC_BUDDY_DECODE_LOW_VALUE_1 );

        REQUIRE( codec_init( BUDDY_CHAN_0_MASK | BUDDY_CHAN_4_MASK, RESOLUTION_CTRL_HIGH) == 0 );
        REQUIRE( codec_decode( (uint8_t *) &buddy_packed_high_cont, &packet ) == CODEC_STATUS_CONTINUE );
        REQUIRE( static_cast<uint16_t>(packet.channels[BUDDY_CHAN_0]) == MAGIC_BUDDY_DECODE_HIGH_VALUE_0 );
        REQUIRE( static_cast<uint16_t>(packet.channels[BUDDY_CHAN_4]) == MAGIC_BUDDY_DECODE_HIGH_VALUE_1 );

        REQUIRE( codec_init( BUDDY_CHAN_0_MASK | BUDDY_CHAN_4_MASK, RESOLUTION_CTRL_SUPER) == 0 );
        REQUIRE( codec_decode( (uint8_t *) &buddy_packed_super_cont, &packet ) == CODEC_STATUS_CONTINUE );
        REQUIRE( static_cast<uint32_t>(packet.channels[BUDDY_CHAN_0]) == MAGIC_BUDDY_DECODE_SUPER_VALUE_0 );
        REQUIRE( static_cast<uint32_t>(packet.channels[BUDDY_CHAN_4]) == MAGIC_BUDDY_DECODE_SUPER_VALUE_1 );
    }

    SECTION( "buddy decode correctly reaches full status for each resolution mode", "" ) {
        clear_channels(packet);

        // number of packets we can fit in frame
        const int MAX_EXPECTED_PACKET_FILLS = MAX_REPORT_SIZE - BUDDY_APP_VALUE_OFFSET - 1;

        std::vector<std::tuple<int, int, int>> expectedResponse = {
            std::make_tuple(RESOLUTION_CTRL_LOW,  BUDDY_DATA_SIZE_LOW, MAX_EXPECTED_PACKET_FILLS),
            std::make_tuple(RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH, (MAX_EXPECTED_PACKET_FILLS / 2)),
            std::make_tuple(RESOLUTION_CTRL_SUPER, BUDDY_DATA_SIZE_SUPER, MAX_EXPECTED_PACKET_FILLS / 4),
        };

        // only activate channel 0
        codec_set_channel_active(BUDDY_CHAN_0, true);

        for (const auto &item : expectedResponse) {
            int res_ctrl = std::get<0>(item);
            int buddy_size = std::get<1>(item);
            int expectedCount = std::get<2>(item);

            REQUIRE( codec_init( BUDDY_CHAN_0_MASK, res_ctrl ) == 0 );

            // initials
            for (int i = 0; i < (expectedCount - 1); i++) {
                test_buffer[BUDDY_APP_INDIC_OFFSET] =  expectedCount;
                REQUIRE( codec_decode((uint8_t *) &test_buffer, &packet) == CODEC_STATUS_CONTINUE );

                if (res_ctrl == RESOLUTION_CTRL_LOW) {
                    REQUIRE( codec_get_offset_count() == (i + 1) );
                } else if (res_ctrl == RESOLUTION_CTRL_HIGH) {
                    REQUIRE( codec_get_offset_count() == ((i + 1) * 2) );
                } else if (res_ctrl == RESOLUTION_CTRL_SUPER) {
                    REQUIRE( codec_get_offset_count() == ((i + 1) * 4) );
                }
            }

            // final
            REQUIRE( codec_decode((uint8_t *) &test_buffer, &packet) == CODEC_STATUS_FULL );
            REQUIRE( codec_get_offset_count() == 0 );
        }
    }
}

TEST_CASE( "buddy host library", "" ) {
    const int FIXED_TIMEOUT_PERIOD = 100; // in msec

    buddy_hid_info_t info;
    clear_context();

    auto ctx = get_context();

    SECTION( "hidapi_init should fail when hid_init returns error return code" ) {
        ctx->rc_hid_init = -1;
        
        hid_device *p = hidapi_init(&info);
        REQUIRE( ctx->fn_hid_init == true );
        REQUIRE( p == nullptr );
    }

    SECTION( "hidapi_init should fail when hid_open returns null pointer for hid_device" ) {
        ctx->rc_hid_open = nullptr;

        hid_device *p = hidapi_init(&info);
        REQUIRE( ctx->fn_hid_open == true );
        REQUIRE( p == nullptr );
        REQUIRE( ctx->param_hid_init_vendor_id == BUDDY_USB_VID );
        REQUIRE( ctx->param_hid_init_product_id == BUDDY_USB_PID );
        REQUIRE( ctx->param_hid_init_serial_number == nullptr );
    }

    SECTION( "hidapi_init should copy fake manufacturer string when invoked" ) {
        ctx->rc_hid_init = 0;
        ctx->rc_hid_open = (hid_device *) malloc(sizeof(hid_device));

        // ANSI C String
        ctx->rc_hid_get_manufacturer_string = -1;
        hid_device *p = hidapi_init(&info);
        REQUIRE( p == ctx->rc_hid_open );

        REQUIRE( strcmp(info.str_mfr, "") == 0 );
        REQUIRE( ctx->fn_hid_init == true );
        REQUIRE( ctx->fn_hid_open == true );
        REQUIRE( ctx->fn_hid_get_manufacturer_string == true );
        cleanup_hid_device(info);

        // Unicode String
        ctx->rc_hid_get_manufacturer_string = 0;
        p = hidapi_init(&info);
        REQUIRE( strcmp(info.str_mfr, HID_INIT_MANUFACTURER_PARAM) == 0 );
        REQUIRE( ctx->fn_hid_init == true );
        REQUIRE( ctx->fn_hid_open == true );
        REQUIRE( ctx->fn_hid_get_manufacturer_string == true );
        cleanup_hid_device(info, p);

        // @todo add free of rc_hid_open here and elsewhere
    }

    SECTION( "hidapi_init should copy fake product string when invoked" ) {
        ctx->rc_hid_init = 0;
        ctx->rc_hid_open = (hid_device *) malloc(sizeof(hid_device));

        // ANSI C String
        ctx->rc_hid_get_product_string = -1;
        hid_device *p = hidapi_init(&info);

        REQUIRE( strcmp(info.str_product, "") == 0 );
        REQUIRE( ctx->fn_hid_init == true );
        REQUIRE( ctx->fn_hid_open == true );
        REQUIRE( ctx->fn_hid_get_product_string == true );
        cleanup_hid_device(info);

        // Unicode String
        ctx->rc_hid_get_product_string = 0;
        p = hidapi_init(&info);
        REQUIRE( strcmp(info.str_product, HID_INIT_PRODUCT_PARAM) == 0 );
        REQUIRE( ctx->fn_hid_init == true );
        REQUIRE( ctx->fn_hid_open == true );
        REQUIRE( ctx->fn_hid_get_product_string == true );
        cleanup_hid_device(info, p);
    }

    SECTION( "hidapi_init should copy serial number string when invoked" ) {
        ctx->rc_hid_init = 0;
        ctx->rc_hid_open = (hid_device *) malloc(sizeof(hid_device));

        // ANSI C String
        ctx->rc_hid_get_serial_number_string = -1;
        hid_device *p = hidapi_init(&info);

        REQUIRE( strcmp(info.str_serial, "") == 0 );
        REQUIRE( ctx->fn_hid_init == true );
        REQUIRE( ctx->fn_hid_open == true );
        REQUIRE( ctx->fn_hid_get_serial_number_string == true );
        cleanup_hid_device(info);

        // Unicode String
        ctx->rc_hid_get_serial_number_string = 0;
        p = hidapi_init(&info);
        REQUIRE( strcmp(info.str_serial, HID_INIT_SERIAL_PARAM) == 0 );
        REQUIRE( ctx->fn_hid_init == true );
        REQUIRE( ctx->fn_hid_open == true );
        REQUIRE( ctx->fn_hid_get_serial_number_string == true );
        cleanup_hid_device(info, p);
    }

    SECTION( "hidapi_init should copy indexed string when invoked" ) {
        ctx->rc_hid_init = 0;
        ctx->rc_hid_open = (hid_device *) malloc(sizeof(hid_device));

        // ANSI C String
        ctx->rc_hid_get_indexed_string = -1;
        hid_device *p = hidapi_init(&info);

        REQUIRE( strcmp(info.str_index_1, "") == 0 );
        REQUIRE( ctx->fn_hid_init == true );
        REQUIRE( ctx->fn_hid_open == true );
        REQUIRE( ctx->fn_hid_get_indexed_string == true );
        cleanup_hid_device(info);

        // Unicode String
        ctx->rc_hid_get_indexed_string = 0;
        p = hidapi_init(&info);
        REQUIRE( strcmp(info.str_index_1, HID_INIT_INDEXED_PARAM) == 0 );
        REQUIRE( ctx->fn_hid_init == true );
        REQUIRE( ctx->fn_hid_open == true );
        REQUIRE( ctx->fn_hid_get_indexed_string == true );
        cleanup_hid_device(info, p);
    }

    SECTION( "hidapi_init should set the device into nonblocking using hidapi nonblock mode when invoked" ) {
        ctx->rc_hid_init = 0;
        ctx->rc_hid_open = (hid_device *) malloc(sizeof(hid_device));

        ctx->rc_hid_get_manufacturer_string = 0;
        ctx->rc_hid_get_product_string = 0;
        ctx->rc_hid_get_serial_number_string = 0;
        ctx->rc_hid_get_indexed_string = 0;

        hid_device *p = hidapi_init(&info);
        cleanup_hid_device(info, p);
    }

    SECTION( "buddy_write_raw should return an error code when buddy_write_packet returns error" ) {
        ctx->rc_hid_write = -1;

        REQUIRE( buddy_write_raw(NULL, 0, 0, NULL, 0) == BUDDY_ERROR_CODE_GENERAL );
    }

    SECTION( "buddy_write_raw should correctly package and OUT HID packet into the frame buffer" ) {
        ctx->rc_hid_write = 0;

        const int MAGIC_APP_CODE_VALUE = 10;
        const int MAGIC_APP_INDIC_VALUE = 20;
        const uint8_t testBufferData[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

        REQUIRE( buddy_write_raw(NULL, MAGIC_APP_CODE_VALUE, MAGIC_APP_INDIC_VALUE, 
                    (unsigned char *) &testBufferData, sizeof(testBufferData)) == BUDDY_ERROR_CODE_OK );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_TYPE_OFFSET] == BUDDY_OUT_DATA_ID );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_CODE_OFFSET] == MAGIC_APP_CODE_VALUE );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_INDIC_OFFSET] == MAGIC_APP_INDIC_VALUE );
        REQUIRE( memcmp(&ctx->outbuf_hid_write[BUDDY_APP_VALUE_OFFSET], &testBufferData, sizeof(testBufferData)) == 0 );
    }

    SECTION( "buddy_write_packet should invoke the hidapi write method to send packet out over USB HID" ) {
        ctx->rc_hid_write = 0;
        const uint8_t testBufferData[] = { 0xFE, 0xED, 0x0F, 0x0E, 0x0D, 0x0C };
        REQUIRE( buddy_write_packet(nullptr, (unsigned char *) &testBufferData, sizeof(testBufferData)) == BUDDY_ERROR_CODE_OK );
        REQUIRE( ctx->fn_hid_write == true );
        REQUIRE( memcmp(&ctx->outbuf_hid_write[0], &testBufferData, sizeof(testBufferData)) == 0 );
    }

    SECTION( "buddy_write_packet should return an error return code when the hid_write method fails with a forced error return code" ) {
        ctx->rc_hid_write = -1;
        REQUIRE( buddy_write_packet(nullptr, nullptr, 0) == BUDDY_ERROR_CODE_GENERAL );
        REQUIRE( ctx->fn_hid_write == true );
    }

    SECTION( "buddy_read_packet should call the hidapi hid_read method" ) {
        ctx->rc_hid_read = 0;

        uint8_t testReadBuffer[MAX_HID_READ_BUFFER_SIZE];
        REQUIRE( buddy_read_packet(nullptr, (unsigned char *) &testReadBuffer, sizeof(testReadBuffer)) == ctx->rc_hid_read );
    }

    SECTION( "buddy_send_generic should pack a single result when streaming is enabled and the codec buffer is not full" ) {
        codec_reset();
        general_packet_t packet;
        packet.channels[BUDDY_CHAN_0] = 1000;

        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );
        REQUIRE( buddy_send_generic(nullptr, &packet, true, 0) == BUDDY_ERROR_CODE_OK );
        REQUIRE( ctx->fn_hid_write == false );
    }

    SECTION( "buddy_send_generic should pack package the frame buffer and issue a hid_write hidapi call when streaming is disabled" ) {
        codec_reset();
        general_packet_t packet;
        packet.channels[BUDDY_CHAN_0] = 1000;

        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );

        const int MAGIC_CODE_VALUE = 100;
        REQUIRE( buddy_send_generic(nullptr, &packet, false, MAGIC_CODE_VALUE) == BUDDY_ERROR_CODE_OK );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_TYPE_OFFSET] == BUDDY_OUT_DATA_ID );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_CODE_OFFSET] == MAGIC_CODE_VALUE );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_INDIC_OFFSET] == 1 );
        REQUIRE( ctx->fn_hid_write == true );
    }

    SECTION( "buddy_read_generic_noblock should return an error code when the underlying hidapi hid_read returns an error code" ) {
        general_packet_t packet;
        ctx->rc_hid_read = -1;
        REQUIRE( buddy_read_generic_noblock(nullptr, &packet, false, 1000) == BUDDY_ERROR_CODE_GENERAL );
    }

    SECTION( "buddy_read_generic_noblock should timeout after the timeout period has elapsed and no HID read packets are availaible" ) {
        general_packet_t packet;
        ctx->rc_hid_read = 0;
        REQUIRE( buddy_read_generic_noblock(nullptr, &packet, false, FIXED_TIMEOUT_PERIOD) == BUDDY_ERROR_CODE_TIMEOUT );
        REQUIRE( ctx->fn_hid_read == true );
    }

    SECTION( "buddy_read_generic_noblock should take fake data buffer and extract remote error code" ) {
        const int FAKE_RETURN_CODE_0 = 0;
        const int FAKE_RETURN_CODE_1 = (uint8_t) -1;
        const int FAKE_RETURN_CODE_2 = 1;

        general_packet_t packet;
        ctx->rc_hid_read = 1;

        ctx->inbuf_hid_read[BUDDY_APP_CODE_OFFSET] |= BUDDY_RESPONSE_TYPE_STATUS; 
        ctx->inbuf_hid_read[BUDDY_APP_INDIC_OFFSET] = FAKE_RETURN_CODE_0;
        REQUIRE( buddy_read_generic_noblock(nullptr, &packet, false, FIXED_TIMEOUT_PERIOD) == FAKE_RETURN_CODE_0 );
        REQUIRE( ctx->fn_hid_read == true );

        ctx->inbuf_hid_read[BUDDY_APP_INDIC_OFFSET] = FAKE_RETURN_CODE_1;
        REQUIRE( buddy_read_generic_noblock(nullptr, &packet, false, FIXED_TIMEOUT_PERIOD) == FAKE_RETURN_CODE_1 );
        
        ctx->inbuf_hid_read[BUDDY_APP_INDIC_OFFSET] = FAKE_RETURN_CODE_2;
        REQUIRE( buddy_read_generic_noblock(nullptr, &packet, false, FIXED_TIMEOUT_PERIOD) == FAKE_RETURN_CODE_2 );
    }

    SECTION( "buddy_read_generic_noblock should return an error invalid status code for unknown BUDDY_APP_CODE_OFFSET value" ) {
        ctx->inbuf_hid_read[BUDDY_APP_CODE_OFFSET] = 0; 
        ctx->rc_hid_read = 1;
        
        general_packet_t packet;
        REQUIRE( buddy_read_generic_noblock(nullptr, &packet, false, FIXED_TIMEOUT_PERIOD) == BUDDY_ERROR_CODE_INVALID );
    }

    SECTION( "buddy_read_generic_noblock should decode the packet" ) {
        ctx->inbuf_hid_read[BUDDY_APP_CODE_OFFSET] = BUDDY_RESPONSE_TYPE_DATA;
        ctx->rc_hid_read = 1;

        codec_reset();

        general_packet_t packet;
        REQUIRE( buddy_read_generic_noblock(nullptr, &packet, false, FIXED_TIMEOUT_PERIOD) == BUDDY_ERROR_CODE_OK );
        REQUIRE( codec_get_offset_count() == 0 );
    }

    SECTION( "buddy_read_generic_noblock should decode the packet channel values correctly" ) {
        REQUIRE( codec_init(BUDDY_CHAN_0_MASK | BUDDY_CHAN_1_MASK, RESOLUTION_CTRL_HIGH) == 0 );

        const uint16_t FIXED_CHANNEL_0_CODED_VALUE = 0x0102;
        const uint16_t FIXED_CHANNEL_1_CODED_VALUE = 0x0304;

        ctx->inbuf_hid_read[BUDDY_APP_CODE_OFFSET] = BUDDY_RESPONSE_TYPE_DATA;
        ctx->rc_hid_read = 1;

        ctx->inbuf_hid_read[BUDDY_APP_CODE_OFFSET] |= BUDDY_RESPONSE_TYPE_STATUS; 
        ctx->inbuf_hid_read[BUDDY_APP_INDIC_OFFSET] = 1;
        ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET] = ((FIXED_CHANNEL_0_CODED_VALUE & 0xFF00) >> 8);
        ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 1] = (FIXED_CHANNEL_0_CODED_VALUE & 0xFF);
        ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 2] = ((FIXED_CHANNEL_1_CODED_VALUE & 0xFF00) >> 8);
        ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 3] = (FIXED_CHANNEL_1_CODED_VALUE & 0xFF);

        general_packet_t packet;
        REQUIRE( buddy_read_generic_noblock(nullptr, &packet, false, FIXED_TIMEOUT_PERIOD) == BUDDY_ERROR_CODE_OK );
        REQUIRE( packet.channels[BUDDY_CHAN_0] == FIXED_CHANNEL_0_CODED_VALUE );
        REQUIRE( packet.channels[BUDDY_CHAN_1] == FIXED_CHANNEL_1_CODED_VALUE );

        codec_reset();
        REQUIRE( buddy_read_generic_noblock(nullptr, &packet, true, FIXED_TIMEOUT_PERIOD) == BUDDY_ERROR_CODE_OK );
        REQUIRE( packet.channels[BUDDY_CHAN_0] == FIXED_CHANNEL_0_CODED_VALUE );
        REQUIRE( packet.channels[BUDDY_CHAN_1] == FIXED_CHANNEL_1_CODED_VALUE );
    }

    SECTION( "buddy_send_pwm should return invalid error code for unrecognized resolution" ) {
        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );

        const uint8_t NO_EXIST_RESOLUTION = (BUDDY_DATA_SIZE_SUPER << 1);
        codec_set_resolution(NO_EXIST_RESOLUTION);

        general_packet_t packet;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_INVALID );
        REQUIRE( buddy_send_pwm(nullptr, &packet, false) == BUDDY_ERROR_CODE_INVALID );
    }

    SECTION( "buddy_send_pwm should return out of bound error code for packet channel values outside acceptable bit resolution range for resolution" ) {
        general_packet_t packet;
        const int DISALLOWED_MIN_VALUE = 0;
        const int DISALLOWED_LOW_MAX_VALUE = 256;
        const int DISALLOWED_HIGH_MAX_VALUE = 65536;

        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );
        packet.channels[0] = DISALLOWED_MIN_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_OUT_OF_BOUND );
        packet.channels[0] = DISALLOWED_LOW_MAX_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_OUT_OF_BOUND );

        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_HIGH) == 0 );
        packet.channels[0] = DISALLOWED_MIN_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_OUT_OF_BOUND );
        packet.channels[0] = DISALLOWED_HIGH_MAX_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_OUT_OF_BOUND );

        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_SUPER) == 0 );
        packet.channels[0] = DISALLOWED_MIN_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_OUT_OF_BOUND );

        // @todo cannot check for out of bound with int32_t.  Don't want to allocate 64-bit though
        // because struct is used on embedded target
    }

    const int ALLOWED_LOW_VALUE = 128;

    SECTION( "buddy_send_pwm should return an invalid error code for unknown pwm_mode values" ) {
        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );

        /// 10 is arbitary, there are only two modes, see RUNTIME_PWM_MODE enum
        const int NONEXIST_PWM_MODE = 10;

        buddy_driver_context *buddyCtx = buddy_get_context();
        buddyCtx->runtime.pwm_mode = NONEXIST_PWM_MODE;

        general_packet_t packet;
        packet.channels[0] = ALLOWED_LOW_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_INVALID );
        REQUIRE( buddy_send_pwm(nullptr, &packet, false) == BUDDY_ERROR_CODE_INVALID );
    }

    SECTION( "buddy_send_pwm should return an invalid error code if a timer count register value cannot be computed" ) {
        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );

        buddy_driver_context *buddyCtx = buddy_get_context();
        buddyCtx->runtime.pwm_mode = RUNTIME_PWM_MODE_FREQUENCY;
        buddyCtx->runtime.pwm_timebase = RUNTIME_PWM_TIMEBASE_TIMER0_OVERFLOW;

        general_packet_t packet;
        packet.channels[0] = ALLOWED_LOW_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_INVALID );
    }
    
    SECTION( "buddy_send_pwm should report out of OK and bound error status code for requested register values that are inside and outside of unsigned 8-bit range") {
        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_SUPER) == 0 );

        buddy_driver_context *buddyCtx = buddy_get_context();

        buddyCtx->runtime.pwm_mode = RUNTIME_PWM_MODE_FREQUENCY;
        buddyCtx->runtime.pwm_timebase = RUNTIME_PWM_TIMEBASE_SYSCLK;

        const int DISALLOWED_PWM_CHANNEL_SYSCLK_VALUE = 10000;
        const int ALLOWED_PWM_CHANNEL_SYSCLK_VALUE = 100000;
        general_packet_t packet;
        packet.channels[0] = DISALLOWED_PWM_CHANNEL_SYSCLK_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_OUT_OF_BOUND );
        packet.channels[0] = ALLOWED_PWM_CHANNEL_SYSCLK_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_OK );

        buddyCtx->runtime.pwm_mode = RUNTIME_PWM_MODE_FREQUENCY;
        buddyCtx->runtime.pwm_timebase = RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_4;

        const int DISALLOWED_PWM_CHANNEL_SYSCLK_DIV_4_VALUE = 1000;
        const int ALLOWED_PWM_CHANNEL_SYSCLK_DIV_4_VALUE = 100000;
        packet.channels[0] = DISALLOWED_PWM_CHANNEL_SYSCLK_DIV_4_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_OUT_OF_BOUND );
        packet.channels[0] = ALLOWED_PWM_CHANNEL_SYSCLK_DIV_4_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_OK );

        buddyCtx->runtime.pwm_mode = RUNTIME_PWM_MODE_FREQUENCY;
        buddyCtx->runtime.pwm_timebase = RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_12;

        const int DISALLOWED_PWM_CHANNEL_SYSCLK_DIV_12_VALUE = 1000;
        const int ALLOWED_PWM_CHANNEL_SYSCLK_DIV_12_VALUE = 10000;
        packet.channels[0] = DISALLOWED_PWM_CHANNEL_SYSCLK_DIV_12_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_OUT_OF_BOUND );
        packet.channels[0] = ALLOWED_PWM_CHANNEL_SYSCLK_DIV_12_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_OK );
    }

    SECTION( "buddy_send_pwm should return an invalid error code for duty cycle PWM with super resolution" ) {
        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_SUPER) == 0 );

        buddy_driver_context *buddyCtx = buddy_get_context();
        buddyCtx->runtime.pwm_mode = RUNTIME_PWM_MODE_DUTY_CYCLE;

        const int DEFAULT_PWM_VALUE = 1234;
        
        general_packet_t packet;
        packet.channels[0] = DEFAULT_PWM_VALUE;
        REQUIRE( buddy_send_pwm(nullptr, &packet, true) == BUDDY_ERROR_CODE_INVALID );
    }

    SECTION( "buddy_send_dac should package an APP_CODE_DAC type packet and send" ) {
        codec_reset();
        general_packet_t packet;
        packet.channels[BUDDY_CHAN_0] = 1000;

        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );
        REQUIRE( buddy_send_dac(nullptr, &packet, false) == BUDDY_ERROR_CODE_OK );

        REQUIRE( ctx->outbuf_hid_write[BUDDY_TYPE_OFFSET] == BUDDY_OUT_DATA_ID );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_CODE_OFFSET] == APP_CODE_DAC );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_INDIC_OFFSET] == 1 );
        REQUIRE( ctx->fn_hid_write == true );

        codec_reset();
        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );
        REQUIRE( buddy_send_dac(nullptr, &packet, true) == BUDDY_ERROR_CODE_OK );
        REQUIRE( codec_get_encode_count() == 1 );
    }

    SECTION( "buddy_read_counter should do a read over HID port and decode the frame buffer into a packet" ) {
        REQUIRE( codec_init(BUDDY_CHAN_0_MASK | BUDDY_CHAN_1_MASK, RESOLUTION_CTRL_SUPER) == 0 );

        const int32_t FIXED_CHANNEL_0_CODED_VALUE = 12345;
        const int32_t FIXED_CHANNEL_1_CODED_VALUE = 45000;

        ctx->inbuf_hid_read[BUDDY_APP_CODE_OFFSET] = BUDDY_RESPONSE_TYPE_DATA;
        ctx->rc_hid_read = 1;

        ctx->inbuf_hid_read[BUDDY_APP_INDIC_OFFSET] = 2;
        
        ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET] = ((FIXED_CHANNEL_0_CODED_VALUE & 0xFF00) >> 24);
        ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 1] = ((FIXED_CHANNEL_0_CODED_VALUE & 0xFF00) >> 16);
        ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 2] = ((FIXED_CHANNEL_0_CODED_VALUE & 0xFF00) >> 8);
        ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 3] = (FIXED_CHANNEL_0_CODED_VALUE & 0xFF);

        ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 4] = ((FIXED_CHANNEL_1_CODED_VALUE & 0xFF00) >> 24);
        ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 5] = ((FIXED_CHANNEL_1_CODED_VALUE & 0xFF00) >> 16);
        ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 6] = ((FIXED_CHANNEL_1_CODED_VALUE & 0xFF00) >> 8);
        ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 7] = (FIXED_CHANNEL_1_CODED_VALUE & 0xFF);

        general_packet_t packet;
        REQUIRE( buddy_read_counter(nullptr, &packet, false) == BUDDY_ERROR_CODE_OK );
        REQUIRE( packet.channels[BUDDY_CHAN_0] == FIXED_CHANNEL_0_CODED_VALUE );
        REQUIRE( packet.channels[BUDDY_CHAN_1] == FIXED_CHANNEL_1_CODED_VALUE );

        codec_reset();

        REQUIRE( buddy_read_counter(nullptr, &packet, true) == BUDDY_ERROR_CODE_OK );
        REQUIRE( packet.channels[BUDDY_CHAN_0] == FIXED_CHANNEL_0_CODED_VALUE );
        REQUIRE( packet.channels[BUDDY_CHAN_1] == FIXED_CHANNEL_1_CODED_VALUE );
    }

    SECTION( "buddy_read_adc should do a read over HID port and decode the frame buffer into a packet" ) {
        const int32_t FIXED_CHANNEL_0_CODED_LOW_RES_VALUE = 64;
        const int32_t FIXED_CHANNEL_1_CODED_LOW_RES_VALUE = 128;
        const int32_t FIXED_CHANNEL_0_CODED_HIGH_RES_VALUE = 512;
        const int32_t FIXED_CHANNEL_1_CODED_HIGH_RES_VALUE = 1024;

        // vector of tuple.  tuple format is as follows:
        //  0: resolution used in initializing the codec
        //  1: channel 0 value placed in raw frame buffer
        //  2: channel 1 value placed in raw frame buffer
        std::vector<std::tuple<uint8_t, int32_t, int32_t>> expectedResponse = {
            std::make_tuple(RESOLUTION_CTRL_LOW, FIXED_CHANNEL_0_CODED_LOW_RES_VALUE, FIXED_CHANNEL_1_CODED_LOW_RES_VALUE),
            std::make_tuple(RESOLUTION_CTRL_HIGH, FIXED_CHANNEL_0_CODED_HIGH_RES_VALUE, FIXED_CHANNEL_1_CODED_HIGH_RES_VALUE),
        };

        for (const auto &item : expectedResponse) {
            uint8_t res_mode = std::get<0>(item);
            int32_t ch0_value = std::get<1>(item);
            int32_t ch1_value = std::get<2>(item);
            
            REQUIRE( codec_init(BUDDY_CHAN_0_MASK | BUDDY_CHAN_1_MASK, res_mode) == 0 );

            ctx->inbuf_hid_read[BUDDY_APP_CODE_OFFSET] = BUDDY_RESPONSE_TYPE_DATA;
            ctx->rc_hid_read = 1;
            ctx->inbuf_hid_read[BUDDY_APP_INDIC_OFFSET] = 2;
        
            if (res_mode == RESOLUTION_CTRL_LOW) {
                ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET] = ch0_value;
                ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 1] = ch1_value;
            } else if (res_mode == RESOLUTION_CTRL_HIGH) {
                ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET] = ((ch0_value & 0xFF00) >> 8);
                ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 1] = (ch0_value & 0xFF);
                ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 2] = ((ch1_value & 0xFF00) >> 8);
                ctx->inbuf_hid_read[BUDDY_APP_VALUE_OFFSET + 3] = (ch1_value & 0xFF);
            }

            general_packet_t packet;
            REQUIRE( buddy_read_adc(nullptr, &packet, false) == BUDDY_ERROR_CODE_OK );
            REQUIRE( packet.channels[BUDDY_CHAN_0] == ch0_value );
            REQUIRE( packet.channels[BUDDY_CHAN_1] == ch1_value );

            codec_reset();

            REQUIRE( buddy_read_adc(nullptr, &packet, true) == BUDDY_ERROR_CODE_OK );
            REQUIRE( packet.channels[BUDDY_CHAN_0] == ch0_value );
            REQUIRE( packet.channels[BUDDY_CHAN_1] == ch1_value );
        }
    }

    SECTION( "buddy_empty should empty the HID read buffer" ) {
        ctx->rc_hid_read = -1;
        REQUIRE( buddy_empty(nullptr) == BUDDY_ERROR_CODE_PROTOCOL );
        
        ctx->rc_hid_read = 0;
        REQUIRE( buddy_empty(nullptr) == BUDDY_ERROR_CODE_OK );
    }

    SECTION( "buddy_clear should return success or failure error code depending on hid_read return code" ) {
        ctx->rc_hid_read = -1;
        REQUIRE( buddy_clear(nullptr) == BUDDY_ERROR_CODE_GENERAL );

        ctx->rc_hid_read = 0;
        REQUIRE( buddy_clear(nullptr) == BUDDY_ERROR_CODE_OK );
    }

    SECTION( "buddy_flush should write any outstanding data in the out buffer buffer" ) {
        const int MAGIC_ENCODE_COUNT = 1;

        buddy_driver_context *buddyCtx = buddy_get_context();
        hid_device handle;

        // dac
        buddyCtx->general.function = GENERAL_CTRL_DAC_ENABLE;
        codec_set_offset_count(1);
        codec_set_encode_count(MAGIC_ENCODE_COUNT);
        
        ctx->rc_hid_write = 0;
        REQUIRE( buddy_flush(&handle) == BUDDY_ERROR_CODE_OK );
        REQUIRE( ctx->fn_hid_write == true );

        REQUIRE( ctx->outbuf_hid_write[BUDDY_TYPE_OFFSET] == BUDDY_OUT_DATA_ID );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_CODE_OFFSET] == APP_CODE_DAC );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_INDIC_OFFSET] == MAGIC_ENCODE_COUNT );
        REQUIRE( ctx->fn_hid_write == true );
        REQUIRE( codec_get_encode_count() == 0 );
        REQUIRE( codec_get_offset_count() == 0 );

        // pwm
        buddyCtx->general.function = GENERAL_CTRL_PWM_ENABLE;
        codec_set_offset_count(1);
        codec_set_encode_count(MAGIC_ENCODE_COUNT);

        ctx->rc_hid_write = 0;
        REQUIRE( buddy_flush(&handle) == BUDDY_ERROR_CODE_OK );
        REQUIRE( ctx->fn_hid_write == true );

        REQUIRE( ctx->outbuf_hid_write[BUDDY_TYPE_OFFSET] == BUDDY_OUT_DATA_ID );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_CODE_OFFSET] == APP_CODE_PWM );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_INDIC_OFFSET] == MAGIC_ENCODE_COUNT );
        REQUIRE( ctx->fn_hid_write == true );
        REQUIRE( codec_get_encode_count() == 0 );
        REQUIRE( codec_get_offset_count() == 0 );

        // unknown
        codec_set_offset_count(1);
        buddyCtx->general.function = GENERAL_CTRL_NONE;
        REQUIRE( buddy_flush(&handle) == BUDDY_ERROR_CODE_GENERAL );
    }

    SECTION( "buddy_flush should return a memory error code when a null handle is provided" ) {
        REQUIRE( buddy_flush(nullptr) == BUDDY_ERROR_CODE_MEMORY );
    }

    SECTION( "buddy_flush should return an error code when the hidapi write routine fails" ) {
        hid_device tempHandle;
 
        codec_set_offset_count(1);

        ctx->rc_hid_write = -1;
        REQUIRE( buddy_flush(&tempHandle) == BUDDY_ERROR_CODE_GENERAL );
    }

    SECTION( "buddy_configure should return an error code when provided null handle, general, runtime, or timing structures" ) {
        hid_device tempHandle;
        ctrl_general_t tempGeneral;
        ctrl_runtime_t tempRuntime;
        ctrl_timing_t tempTiming;
        
        REQUIRE( buddy_configure(nullptr, &tempGeneral, &tempRuntime, &tempTiming) == BUDDY_ERROR_CODE_MEMORY );
        REQUIRE( buddy_configure(&tempHandle, nullptr, &tempRuntime, &tempTiming) == BUDDY_ERROR_CODE_MEMORY );
        REQUIRE( buddy_configure(&tempHandle, &tempGeneral, nullptr, &tempTiming) == BUDDY_ERROR_CODE_MEMORY );
        REQUIRE( buddy_configure(&tempHandle, &tempGeneral, &tempRuntime, nullptr) == BUDDY_ERROR_CODE_MEMORY );
    }

    SECTION( "buddy_configure should copy the general, runtime, and timing structures into internal context structure" ) {
        ctrl_general_t tempGeneral;
        ctrl_runtime_t tempRuntime;
        ctrl_timing_t tempTiming;
        hid_device tempHandle;

        const int DEFAULT_SAMPLE_RATE = 10000;
        const int DEFAULT_AVERAGING = 1;

        tempGeneral.function = GENERAL_CTRL_ADC_ENABLE;
        tempGeneral.mode = MODE_CTRL_IMMEDIATE;
        tempGeneral.channel_mask = BUDDY_CHAN_1_MASK;
        tempGeneral.resolution = RESOLUTION_CTRL_HIGH;

        uint32_t origPeriodTime = (uint32_t) FREQUENCY_TO_NSEC(DEFAULT_SAMPLE_RATE);
	    tempTiming.period = origPeriodTime;
	    tempTiming.averaging = DEFAULT_AVERAGING;

	    tempRuntime.adc_ref = RUNTIME_ADC_REF_VDD;

        REQUIRE( buddy_configure(&tempHandle, &tempGeneral, &tempRuntime, &tempTiming) == BUDDY_ERROR_CODE_OK );
    
        buddy_driver_context *buddyCtx = buddy_get_context();

        REQUIRE( buddyCtx->general.function == GENERAL_CTRL_ADC_ENABLE );
        REQUIRE( buddyCtx->general.mode == MODE_CTRL_IMMEDIATE );
        REQUIRE( buddyCtx->general.channel_mask == BUDDY_CHAN_1_MASK );
        REQUIRE( buddyCtx->general.resolution == RESOLUTION_CTRL_HIGH );

        // @todo this doesn't work when you wrap in the swap_uint32
        // REQUIRE( buddyCtx->timing.period == (origPeriodTime / codec_get_channel_count()) );

        // special case for counter mode
        tempGeneral.function = GENERAL_CTRL_COUNTER_ENABLE;
        REQUIRE( buddy_configure(&tempHandle, &tempGeneral, &tempRuntime, &tempTiming) == BUDDY_ERROR_CODE_OK );
        REQUIRE( buddyCtx->timing.period == swap_uint32(origPeriodTime) );
    }

    SECTION( "buddy_configure should initialize the codec with the proper resolution and active channel configuration" ) {
        ctrl_general_t tempGeneral;
        ctrl_runtime_t tempRuntime;
        ctrl_timing_t tempTiming;
        hid_device tempHandle;
        
        tempGeneral.function = GENERAL_CTRL_ADC_ENABLE;
        tempGeneral.mode = MODE_CTRL_IMMEDIATE;
        tempGeneral.channel_mask = BUDDY_CHAN_1_MASK;
        tempGeneral.resolution = RESOLUTION_CTRL_HIGH;

        REQUIRE( buddy_configure(&tempHandle, &tempGeneral, &tempRuntime, &tempTiming) == BUDDY_ERROR_CODE_OK );
        REQUIRE( codec_get_resolution() == RESOLUTION_CTRL_HIGH );
        REQUIRE( codec_get_channel_count() == 1 );
    }


    SECTION( "buddy_configure should return an error code when the write using hidapi hid_write fails" ) {
        ctrl_general_t tempGeneral;
        ctrl_runtime_t tempRuntime;
        ctrl_timing_t tempTiming;
        hid_device tempHandle;

        tempGeneral.function = GENERAL_CTRL_ADC_ENABLE;
        tempGeneral.mode = MODE_CTRL_IMMEDIATE;
        tempGeneral.channel_mask = BUDDY_CHAN_1_MASK;
        tempGeneral.resolution = RESOLUTION_CTRL_HIGH;

        ctx->rc_hid_write = -1;
        REQUIRE( buddy_configure(&tempHandle, &tempGeneral, &tempRuntime, &tempTiming) == BUDDY_ERROR_CODE_GENERAL );
    }

    SECTION( "buddy_write_config should send a configuration packet to the device when we check the hidapi hid_write output buffer" ) {
        ctrl_general_t tempGeneral;
        ctrl_runtime_t tempRuntime;
        ctrl_timing_t tempTiming;
        hid_device handle;

        // runtime
        ctx->fn_hid_write = false;
        ctx->rc_hid_write = 0;
        REQUIRE( buddy_write_config(&handle, CTRL_RUNTIME, (uint8_t *) &tempRuntime, sizeof(tempRuntime)) == BUDDY_ERROR_CODE_OK );
        REQUIRE( ctx->fn_hid_write == true );

        REQUIRE( ctx->outbuf_hid_write[BUDDY_TYPE_OFFSET] == BUDDY_OUT_DATA_ID );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_CODE_OFFSET] == APP_CODE_CTRL );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_INDIC_OFFSET] == CTRL_RUNTIME );

        // general
        ctx->fn_hid_write = false;
        ctx->rc_hid_write = 0;
        REQUIRE( buddy_write_config(&handle, CTRL_GENERAL, (uint8_t *) &tempGeneral, sizeof(tempGeneral)) == BUDDY_ERROR_CODE_OK );
        REQUIRE( ctx->fn_hid_write == true );

        REQUIRE( ctx->outbuf_hid_write[BUDDY_TYPE_OFFSET] == BUDDY_OUT_DATA_ID );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_CODE_OFFSET] == APP_CODE_CTRL );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_INDIC_OFFSET] == CTRL_GENERAL );

        // timing
        ctx->fn_hid_write = false;
        ctx->rc_hid_write = 0;
        REQUIRE( buddy_write_config(&handle, CTRL_TIMING, (uint8_t *) &tempTiming, sizeof(tempTiming)) == BUDDY_ERROR_CODE_OK );
        REQUIRE( ctx->fn_hid_write == true );

        REQUIRE( ctx->outbuf_hid_write[BUDDY_TYPE_OFFSET] == BUDDY_OUT_DATA_ID );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_CODE_OFFSET] == APP_CODE_CTRL );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_INDIC_OFFSET] == CTRL_TIMING );
    }

    SECTION( "buddy_write_config should return an error code when the hidapi hid_write method fails" ) {
        ctrl_timing_t tempTiming;
        hid_device handle;

        ctx->rc_hid_write = -1;
        REQUIRE( buddy_write_config(&handle, CTRL_TIMING, (uint8_t *) &tempTiming, sizeof(tempTiming)) != BUDDY_ERROR_CODE_OK );
    }

    const int FIXED_BUFFER_SIZE = 64;

    SECTION( "buddy_get_response should return an error code when an invalid buffer pointer or length are provided as arguments" ) {
        uint8_t buffer[FIXED_BUFFER_SIZE];

        REQUIRE( buddy_get_response(nullptr, nullptr, FIXED_BUFFER_SIZE) == BUDDY_ERROR_CODE_GENERAL );
        REQUIRE( buddy_get_response(nullptr, static_cast<uint8_t *>(&buffer[0]), 0) == BUDDY_ERROR_CODE_GENERAL );
    }

    SECTION( "buddy_get_response should return a protocol error code when the hidapi hid_read routine fails" ) {
        ctx->rc_hid_read = -1;
        
        uint8_t buffer[FIXED_BUFFER_SIZE];
        REQUIRE( buddy_get_response(nullptr, static_cast<uint8_t *>(&buffer[0]), sizeof(buffer))  == BUDDY_ERROR_CODE_PROTOCOL );
    }

    SECTION( "buddy_get_response should return a timeout return code when the a fixed number of HID IN messages are checked and response frame not found" ) {
        ctx->rc_hid_read = 0;

        uint8_t buffer[FIXED_BUFFER_SIZE];
        REQUIRE( buddy_get_response(nullptr, static_cast<uint8_t *>(&buffer[0]), sizeof(buffer)) == BUDDY_ERROR_CODE_TIMEOUT );
    }

    SECTION( "buddy_get_response should return the remote error code return value when a BUDDY_RESPONSE_TYPE_STATUS type is presented" ) {
        ctx->rc_hid_read = 1;

        const int ERROR_CODE_TRY_1 = 1;
        const int ERROR_CODE_TRY_2 = 2;
        const int ERROR_CODE_TRY_3 = -1;

        uint8_t buffer[FIXED_BUFFER_SIZE];

        ctx->inbuf_hid_read[BUDDY_APP_CODE_OFFSET] = BUDDY_RESPONSE_TYPE_STATUS;

        ctx->inbuf_hid_read[BUDDY_APP_INDIC_OFFSET] = ERROR_CODE_TRY_1;
        REQUIRE( buddy_get_response(nullptr, static_cast<uint8_t *>(&buffer[0]), sizeof(buffer)) == ERROR_CODE_TRY_1 );
        ctx->inbuf_hid_read[BUDDY_APP_INDIC_OFFSET] = ERROR_CODE_TRY_2;
        REQUIRE( buddy_get_response(nullptr, static_cast<uint8_t *>(&buffer[0]), sizeof(buffer)) == ERROR_CODE_TRY_2 );
        ctx->inbuf_hid_read[BUDDY_APP_INDIC_OFFSET] = ERROR_CODE_TRY_3;
        REQUIRE( buddy_get_response(nullptr, static_cast<uint8_t *>(&buffer[0]), sizeof(buffer)) == ERROR_CODE_TRY_3 );
    }

    SECTION( "buddy_get_response should copy the HID read input buffer into the " ) {
        ctx->rc_hid_read = 1;

        uint8_t buffer[FIXED_BUFFER_SIZE] = { 0xBE, 0xEF, 0xF0, 0x0D, 0x01, 0x02 };
        uint8_t read_buffer[FIXED_BUFFER_SIZE];
        memcpy( &ctx->inbuf_hid_read[BUDDY_APP_INDIC_OFFSET], &buffer, sizeof(buffer));
        
        //memcpy(buffer, &in_buf[BUDDY_APP_INDIC_OFFSET], copy_length);
        
        ctx->inbuf_hid_read[BUDDY_APP_CODE_OFFSET] = BUDDY_RESPONSE_TYPE_DATA;
        REQUIRE( buddy_get_response(nullptr, static_cast<uint8_t *>(&read_buffer[0]), sizeof(read_buffer)) == BUDDY_ERROR_CODE_OK );
        REQUIRE( memcmp(&read_buffer[0], &buffer[0], FIXED_BUFFER_SIZE) == 0 );
    }

    SECTION( "buddy_reset_device should issue an HID write packet with an app code of APP_CODE_RESET" ) {
        ctx->fn_hid_write = false;
        ctx->rc_hid_write = 0;

        hid_device handle;
        REQUIRE( buddy_reset_device(&handle) == BUDDY_ERROR_CODE_OK );
        REQUIRE( ctx->fn_hid_write == true );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_TYPE_OFFSET] == BUDDY_OUT_DATA_ID );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_CODE_OFFSET] == APP_CODE_RESET );
    }
    
    SECTION( "buddy_get_firmware_info should return a memory error return code if handle or firmware info pointer are null" ) {
        hid_device tempHandle;
        firmware_info_t fwInfo;

        REQUIRE( buddy_get_firmware_info(nullptr, &fwInfo) == BUDDY_ERROR_CODE_MEMORY );
        REQUIRE( buddy_get_firmware_info(&tempHandle, nullptr) == BUDDY_ERROR_CODE_MEMORY );
    }

    SECTION( "buddy_get_firmware_info should return an error code if the info request cannot be written or response read" ) {
        hid_device tempHandle;
        firmware_info_t fwInfo;

        ctx->rc_hid_write = -1;
        REQUIRE( buddy_get_firmware_info(&tempHandle, &fwInfo) == BUDDY_ERROR_CODE_GENERAL );

        ctx->rc_hid_write = 1;
        ctx->rc_hid_read = -1;
        REQUIRE( buddy_get_firmware_info(&tempHandle, &fwInfo) == BUDDY_ERROR_CODE_GENERAL );
    }

    SECTION( "buddy_get_firmware_info should read an HID IN packet and copy the firmare info structure" ) {
        ctx->rc_hid_read = 1;
        ctx->rc_hid_write = 1;

        const uint32_t FW_INFO_SERIAL = 0x12345678;
        const uint32_t FW_INFO_FLASH_DATETIME = 0xDEADBEEF;
        const uint8_t FW_INFO_REV_MAJOR = 1;
        const uint8_t FW_INFO_REV_MINOR = 2;
        const uint8_t FW_INFO_REV_TINY = 3;
        const uint8_t FW_INFO_BOOTL_REV_MAJOR = 4;
        const uint8_t FW_INFO_BOOTL_REV_MINOR = 5;
        const uint8_t FW_INFO_BOOTL_REV_TINY = 6;
        const uint8_t FW_INFO_TYPE_DAC = 7;

        firmware_info_t fwInfo;
        fwInfo.serial = FW_INFO_SERIAL;
        fwInfo.flash_datetime = FW_INFO_FLASH_DATETIME;
        fwInfo.fw_rev_major = FW_INFO_REV_MAJOR;
        fwInfo.fw_rev_minor = FW_INFO_REV_MINOR;
        fwInfo.fw_rev_tiny = FW_INFO_REV_TINY;
        fwInfo.bootl_rev_major = FW_INFO_BOOTL_REV_MAJOR;
        fwInfo.bootl_rev_minor = FW_INFO_BOOTL_REV_MINOR;
        fwInfo.bootl_rev_tiny = FW_INFO_BOOTL_REV_TINY;
        fwInfo.type_dac = FW_INFO_TYPE_DAC;

        memcpy( &ctx->inbuf_hid_read[BUDDY_APP_INDIC_OFFSET], (void *) &fwInfo, sizeof(fwInfo));
        //memcpy(buffer, &in_buf[BUDDY_APP_INDIC_OFFSET], copy_length);
        
        ctx->inbuf_hid_read[BUDDY_APP_CODE_OFFSET] = BUDDY_RESPONSE_TYPE_DATA;
        
        hid_device tempHandle;
        firmware_info_t readFwInfo;
        REQUIRE( buddy_get_firmware_info(&tempHandle, &readFwInfo) == BUDDY_ERROR_CODE_OK );

        REQUIRE( readFwInfo.serial == readFwInfo.serial );
        REQUIRE( readFwInfo.flash_datetime == readFwInfo.flash_datetime );
        REQUIRE( readFwInfo.fw_rev_major == readFwInfo.fw_rev_major );
        REQUIRE( readFwInfo.fw_rev_minor == readFwInfo.fw_rev_minor );
        REQUIRE( readFwInfo.fw_rev_tiny == readFwInfo.fw_rev_tiny );
        REQUIRE( readFwInfo.bootl_rev_major == readFwInfo.bootl_rev_major );
        REQUIRE( readFwInfo.bootl_rev_minor == readFwInfo.bootl_rev_minor );
        REQUIRE( readFwInfo.bootl_rev_tiny == readFwInfo.bootl_rev_tiny );
        REQUIRE( readFwInfo.type_dac == readFwInfo.type_dac );
    }

    SECTION( "buddy_init should return a null handle when the hid_init or hid_open fails" ) {
        ctx->rc_hid_init = -1;
        
        buddy_hid_info_t hidInfo = { 0 };
        firmware_info_t fwInfo = { 0 };

        hid_device *p = buddy_init(&hidInfo, &fwInfo);

        REQUIRE( ctx->fn_hid_init == true );
        REQUIRE( p == nullptr );
        REQUIRE( hidInfo.str_index_1 == nullptr );
        REQUIRE( hidInfo.str_mfr == nullptr );
        REQUIRE( hidInfo.str_product == nullptr );
        REQUIRE( hidInfo.str_serial == nullptr );

        ctx->rc_hid_init = 0;
        ctx->rc_hid_open = nullptr;

        REQUIRE( buddy_init(&hidInfo, &fwInfo) == nullptr );
    }

    SECTION( "buddy_init should allocate storage for hidapi device string fields" ) {
        ctx->rc_hid_init = 0;
        // just so dummy address here to get passed invalid handle check
        ctx->rc_hid_open = reinterpret_cast<hid_device*>(0x123456);

        // empty strings
        ctx->rc_hid_get_manufacturer_string = -1;
        ctx->rc_hid_get_product_string = -1;
        ctx->rc_hid_get_serial_number_string = -1;
        ctx->rc_hid_get_indexed_string = -1;

        buddy_hid_info_t hidInfo = { 0 };
        firmware_info_t fwInfo = { 0 };

        hid_device *p = buddy_init(&hidInfo, &fwInfo);

        REQUIRE( ctx->fn_hid_init == true );

        REQUIRE( hidInfo.str_index_1 != nullptr );
        REQUIRE( hidInfo.str_mfr != nullptr );
        REQUIRE( hidInfo.str_product != nullptr );
        REQUIRE( hidInfo.str_serial != nullptr );

        REQUIRE( strcmp(hidInfo.str_index_1, "") == 0 );
        REQUIRE( strcmp(hidInfo.str_mfr, "") == 0 );
        REQUIRE( strcmp(hidInfo.str_product, "") == 0 );
        REQUIRE( strcmp(hidInfo.str_serial, "") == 0 );

        free(hidInfo.str_index_1);
        free(hidInfo.str_mfr);
        free(hidInfo.str_serial);
        free(hidInfo.str_product);

        // wide char string
        ctx->rc_hid_get_manufacturer_string = 0;
        ctx->rc_hid_get_product_string = 0;
        ctx->rc_hid_get_serial_number_string = 0;
        ctx->rc_hid_get_indexed_string = 0;
        ctx->fn_hid_init = false; // reset

        memset(&hidInfo, 0, sizeof(hidInfo));
        memset(&fwInfo, 0, sizeof(hidInfo));

        p = buddy_init(&hidInfo, &fwInfo);

        REQUIRE( ctx->fn_hid_init == true );

        REQUIRE( hidInfo.str_index_1 != nullptr );
        REQUIRE( hidInfo.str_mfr != nullptr );
        REQUIRE( hidInfo.str_product != nullptr );
        REQUIRE( hidInfo.str_serial != nullptr );

        REQUIRE( strcmp(hidInfo.str_index_1, HID_INIT_INDEXED_PARAM) == 0 );
        REQUIRE( strcmp(hidInfo.str_mfr, HID_INIT_MANUFACTURER_PARAM) == 0 );
        REQUIRE( strcmp(hidInfo.str_product, HID_INIT_PRODUCT_PARAM) == 0 );
        REQUIRE( strcmp(hidInfo.str_serial, HID_INIT_SERIAL_PARAM) == 0 );

        free(hidInfo.str_index_1);
        free(hidInfo.str_mfr);
        free(hidInfo.str_serial);
        free(hidInfo.str_product);
    }

    SECTION( "buddy_init should place the hidapi connect in nonblocking mode" ) {
        ctx->rc_hid_init = 0;
        // just so dummy address here to get passed invalid handle check
        ctx->rc_hid_open = reinterpret_cast<hid_device*>(0x123456);

        // empty strings
        ctx->rc_hid_get_manufacturer_string = -1;
        ctx->rc_hid_get_product_string = -1;
        ctx->rc_hid_get_serial_number_string = -1;
        ctx->rc_hid_get_indexed_string = -1;

        buddy_hid_info_t hidInfo = { 0 };
        firmware_info_t fwInfo = { 0 };

        REQUIRE( buddy_init(&hidInfo, &fwInfo) != nullptr );
        REQUIRE( ctx->fn_hid_set_nonblocking == true );
        REQUIRE( ctx->nonblocking == 1 );

        free(hidInfo.str_index_1);
        free(hidInfo.str_mfr);
        free(hidInfo.str_serial);
        free(hidInfo.str_product);
    }

    SECTION( "buddy_cleanup should return an error code for an invalid handle or hid info structure" ) {
        buddy_hid_info_t hidInfo;
        hid_device hidDevice;

        REQUIRE( buddy_cleanup(nullptr, &hidInfo, true) == BUDDY_ERROR_CODE_MEMORY );
        REQUIRE( buddy_cleanup(&hidDevice, nullptr, true) == BUDDY_ERROR_CODE_MEMORY );
    }

    const int DEFAULT_MALLOC_SIZE = 100;

    SECTION( "buddy_cleanup should free the allocated USB device info buffer filled in by hidapi" ) {
        buddy_hid_info_t hidInfo;
        hid_device *hidDevice;

        hidInfo.str_mfr = static_cast<char *>(malloc(DEFAULT_MALLOC_SIZE));
        hidInfo.str_product = static_cast<char *>(malloc(DEFAULT_MALLOC_SIZE));
        hidInfo.str_serial = static_cast<char *>(malloc(DEFAULT_MALLOC_SIZE));
        hidInfo.str_index_1 = static_cast<char *>(malloc(DEFAULT_MALLOC_SIZE));

        hidDevice = static_cast<hid_device *>(malloc(sizeof(hid_device)));
        REQUIRE( buddy_cleanup(hidDevice, &hidInfo, true) == BUDDY_ERROR_CODE_OK );

        REQUIRE( hidInfo.str_mfr == nullptr );
        REQUIRE( hidInfo.str_product == nullptr );
        REQUIRE( hidInfo.str_serial == nullptr );
        REQUIRE( hidInfo.str_index_1 == nullptr );

        REQUIRE( ctx->fn_hid_write == true );
        REQUIRE( ctx->fn_hid_close == true );
        REQUIRE( ctx->fn_hid_exit == true );

        REQUIRE( ctx->outbuf_hid_write[BUDDY_TYPE_OFFSET] == BUDDY_OUT_DATA_ID );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_CODE_OFFSET] == APP_CODE_CTRL );
        REQUIRE( ctx->outbuf_hid_write[BUDDY_APP_INDIC_OFFSET] == CTRL_GENERAL );

        ctrl_general_t *general = reinterpret_cast<ctrl_general_t *>(&ctx->outbuf_hid_write[BUDDY_APP_VALUE_OFFSET]);
        REQUIRE( general->function == GENERAL_CTRL_NONE );
    }

    SECTION( "buddy_cleanup should return an error code when the raw write fails" ) {
        buddy_hid_info_t hidInfo;
        hid_device hidDevice;

        ctx->rc_hid_write = -1;
        hidInfo.str_mfr = static_cast<char *>(malloc(DEFAULT_MALLOC_SIZE));
        hidInfo.str_product = static_cast<char *>(malloc(DEFAULT_MALLOC_SIZE));
        hidInfo.str_serial = static_cast<char *>(malloc(DEFAULT_MALLOC_SIZE));
        hidInfo.str_index_1 = static_cast<char *>(malloc(DEFAULT_MALLOC_SIZE));
        REQUIRE( buddy_cleanup(&hidDevice, &hidInfo, true) == BUDDY_ERROR_CODE_GENERAL );
    }
}