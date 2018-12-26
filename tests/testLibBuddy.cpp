#define CATCH_CONFIG_MAIN

#include "catch.hpp"

extern "C" {
#include "buddy_common.h"
#include "codec.h"
#include "support.h"
#include "utility.h"
}

#include <vector>
#include <tuple>

#include <stdio.h>
#include <sys/timeb.h> 

extern uint8_t _data_size;

void clear_channels(general_packet_t &packet) {
    for (int i = BUDDY_CHAN_0; i < BUDDY_CHAN_7; i++) {
        packet.channels[i] = 0;
        codec_set_channel_active(i, false);
    }
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

TEST_CASE( "buddy frame encoder packages packets correctly", "" ) {
    uint8_t test_buffer[MAX_REPORT_SIZE];
    general_packet_t packet;

    SECTION( "buddy encode rejects invalid parameters", "" ) {
        REQUIRE( codec_init(BUDDY_CHAN_0_MASK, RESOLUTION_CTRL_LOW) == 0 );

        REQUIRE( codec_encode(NULL, (general_packet_t *) &packet) == CODEC_STATUS_ERROR );
        REQUIRE( codec_encode( (uint8_t *) &test_buffer, NULL) == CODEC_STATUS_ERROR );
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

    SECTION( "buddy decode rejects invalid parameters", "" ) {
        REQUIRE( codec_decode(NULL, (general_packet_t *) &packet) == CODEC_STATUS_ERROR );
        REQUIRE( codec_decode( (uint8_t *) &test_buffer, NULL) == CODEC_STATUS_ERROR );
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