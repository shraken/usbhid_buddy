#define CATCH_CONFIG_MAIN

#include "catch.hpp"

extern "C" {
#include "support.h"
#include "utility.h"
#include "usbhid_buddy.h"
}

#include <vector>
#include <tuple>

#include <stdio.h>
#include <sys/timeb.h> 

extern uint8_t encode_count;
extern uint8_t decode_count;
extern uint8_t codec_byte_offset;

extern uint8_t _resolution_mode;
extern uint8_t _chan_enable[BUDDY_CHAN_LENGTH];
extern uint8_t _data_size;

void clear_channels(general_packet_t &packet) {
    for (int i = BUDDY_CHAN_0; i < BUDDY_CHAN_7; i++) {
        packet.channels[i] = 0;
        _chan_enable[i] = 0;
    }
}

void reset_state(int resolution_mode, int data_size) {
    codec_byte_offset = 0;
    _resolution_mode = resolution_mode;
    _data_size = data_size;
}

void setup_fake_decode_data(uint8_t *buffer, uint8_t resolution_mode) {
    int i;

    if (_resolution_mode == RESOLUTION_CTRL_LOW) {
        for (i = 0; i < MAX_REPORT_SIZE; i++) {
            *(buffer + i) = (uint8_t) i;
        }
    } else if (_resolution_mode == RESOLUTION_CTRL_HIGH) {
        for (i = 0; i < (MAX_REPORT_SIZE / sizeof(uint16_t)); i++) {
            *(buffer + (i * sizeof(uint16_t))) = (uint16_t) i;
        }
    } else if (_resolution_mode == RESOLUTION_CTRL_SUPER) {
        for (i = 0; i < (MAX_REPORT_SIZE / sizeof(uint32_t)); i++) {
            *(buffer + (i * sizeof(uint32_t))) = (uint32_t) i;
        }
    } else {
        return;
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
    reset_codec();

    REQUIRE( codec_byte_offset == 0 );
    REQUIRE( encode_count == 0 );
    REQUIRE( decode_count == 0 );
}

TEST_CASE( "buddy frame encoder packages packets correctly", "" ) {
    uint8_t test_buffer[MAX_REPORT_SIZE];
    general_packet_t packet;

    SECTION( "buddy encode rejects invalid parameters", "" ) {
        REQUIRE( encode(NULL, (general_packet_t *) &packet) == CODEC_STATUS_ERROR );
        REQUIRE( encode( (uint8_t *) &test_buffer, NULL) == CODEC_STATUS_ERROR );
    }

    SECTION( "buddy encode correctly downscales packet values for lower resolutions", "" ) {
        clear_channels(packet);

        // expected response from the encode function.  the encode method
        // should downscale the resolution by bit shift when a larger
        // resolution is being packed in a lower resolution
        //
        // resolution_mode, data_size, packet value, encoded value
        std::vector<std::tuple<int, int, uint32_t, uint32_t>> expectedResponse = {
            { RESOLUTION_CTRL_LOW, BUDDY_DATA_SIZE_LOW, 0xFFFFFF, 0xFF },
            { RESOLUTION_CTRL_LOW, BUDDY_DATA_SIZE_LOW, 0xFFFF, 0xFF },
            { RESOLUTION_CTRL_LOW, BUDDY_DATA_SIZE_LOW, 0xFF, 0xFF },
            { RESOLUTION_CTRL_LOW, BUDDY_DATA_SIZE_LOW, 0x7F, 0x7F },
            { RESOLUTION_CTRL_LOW, BUDDY_DATA_SIZE_LOW, 0x00, 0x00 },

            { RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH, 0xFFFFFFFF, 0xFFFF },
            { RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH, 0xFFFF, 0xFFFF },
            { RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH, 0x7FFF, swap_uint16(0x7FFF) },
            { RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH, 0x0000, 0x0000 },

            { RESOLUTION_CTRL_SUPER, BUDDY_DATA_SIZE_SUPER, 0xFFFFFFFF, 0xFFFFFFFF },
            { RESOLUTION_CTRL_SUPER, BUDDY_DATA_SIZE_SUPER, 0x7FFFFFFF, swap_uint32(0x7FFFFFFF) },
            { RESOLUTION_CTRL_SUPER, BUDDY_DATA_SIZE_SUPER, 0x00, 0x00 },
        };

        for (const auto &item : expectedResponse) {
            int res_mode = std::get<0>(item);
            int data_size = std::get<1>(item);
            uint32_t in_value = std::get<2>(item);
            uint32_t out_value_expect = std::get<3>(item);

            packet.channels[BUDDY_CHAN_0] = in_value;
            _chan_enable[BUDDY_CHAN_0] = 1;

            reset_state(res_mode, data_size);

            int old_codec_byte_offset = codec_byte_offset;
            REQUIRE( encode((uint8_t *) &test_buffer, &packet) == CODEC_STATUS_CONTINUE );
            REQUIRE( codec_byte_offset == (old_codec_byte_offset + data_size) );
            
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
            { RESOLUTION_CTRL_LOW,  BUDDY_DATA_SIZE_LOW, MAX_EXPECTED_PACKET_FILLS },
            { RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH, MAX_EXPECTED_PACKET_FILLS / 2 },
            { RESOLUTION_CTRL_SUPER, BUDDY_DATA_SIZE_SUPER, MAX_EXPECTED_PACKET_FILLS / 4 },
        };

        _chan_enable[BUDDY_CHAN_0] = 1; // only activate channel 0

        for (const auto &item : expectedResponse) {
            int res_ctrl = std::get<0>(item);
            int buddy_size = std::get<1>(item);
            int expectedCount = std::get<2>(item);

            reset_state(res_ctrl, buddy_size);

            // initials
            for (int i = 0; i <= (expectedCount - 1); i++) {
                REQUIRE( encode((uint8_t *) &test_buffer, &packet) == CODEC_STATUS_CONTINUE );

                if (res_ctrl == RESOLUTION_CTRL_LOW) {
                    REQUIRE( codec_byte_offset == (i + 1) );
                } else if (res_ctrl == RESOLUTION_CTRL_HIGH) {
                    REQUIRE( codec_byte_offset == ((i + 1) * 2) );
                } else if (res_ctrl == RESOLUTION_CTRL_SUPER) {
                    REQUIRE( codec_byte_offset == ((i + 1) * 4) );
                }
            }

            // final
            REQUIRE( encode((uint8_t *) &test_buffer, &packet) == CODEC_STATUS_FULL );
            REQUIRE( codec_byte_offset == 0 );
        }
    }

    SECTION( "buddy encode correctly rejects invalid resolution modes", "" ) {
        _resolution_mode = RESOLUTION_CTRL_END_MARKER;

        REQUIRE( encode((uint8_t *) &test_buffer, &packet) == CODEC_STATUS_ERROR );
    }
}

TEST_CASE( "buddy frame decoder parses frame to packets correctly", "" ) {
    uint8_t test_buffer[MAX_REPORT_SIZE];
    general_packet_t packet;

    SECTION( "buddy decode rejects invalid parameters", "" ) {
        REQUIRE( decode(NULL, (general_packet_t *) &packet) == CODEC_STATUS_ERROR );
        REQUIRE( decode( (uint8_t *) &test_buffer, NULL) == CODEC_STATUS_ERROR );
    }

    SECTION( "buddy decode correctly converts for the various resolutions", "" ) {
        clear_channels(packet);

        _chan_enable[BUDDY_CHAN_0] = 1;
        _chan_enable[BUDDY_CHAN_4] = 1;

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

        reset_state(RESOLUTION_CTRL_LOW, BUDDY_DATA_SIZE_LOW);
        reset_codec();
        REQUIRE( decode( (uint8_t *) &buddy_packed_low_cont, &packet) == CODEC_STATUS_CONTINUE );
        REQUIRE( packet.channels[BUDDY_CHAN_0] == MAGIC_BUDDY_DECODE_LOW_VALUE_0 );
        REQUIRE( packet.channels[BUDDY_CHAN_4] == MAGIC_BUDDY_DECODE_LOW_VALUE_1 );

        reset_state(RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH);
        reset_codec();
        REQUIRE( decode( (uint8_t *) &buddy_packed_high_cont, &packet ) == CODEC_STATUS_CONTINUE );
        REQUIRE( static_cast<uint16_t>(packet.channels[BUDDY_CHAN_0]) == MAGIC_BUDDY_DECODE_HIGH_VALUE_0 );
        REQUIRE( static_cast<uint16_t>(packet.channels[BUDDY_CHAN_4]) == MAGIC_BUDDY_DECODE_HIGH_VALUE_1 );

        reset_state(RESOLUTION_CTRL_SUPER, BUDDY_DATA_SIZE_SUPER);
        reset_codec();
        REQUIRE( decode( (uint8_t *) &buddy_packed_super_cont, &packet ) == CODEC_STATUS_CONTINUE );
        REQUIRE( static_cast<uint32_t>(packet.channels[BUDDY_CHAN_0]) == MAGIC_BUDDY_DECODE_SUPER_VALUE_0 );
        REQUIRE( static_cast<uint32_t>(packet.channels[BUDDY_CHAN_4]) == MAGIC_BUDDY_DECODE_SUPER_VALUE_1 );
    }

    SECTION( "buddy decode correctly reaches full status for each resolution mode", "" ) {
        clear_channels(packet);

        // number of packets we can fit in frame
        const int MAX_EXPECTED_PACKET_FILLS = MAX_REPORT_SIZE - BUDDY_APP_VALUE_OFFSET - 1;

        std::vector<std::tuple<int, int, int>> expectedResponse = {
            { RESOLUTION_CTRL_LOW,  BUDDY_DATA_SIZE_LOW, MAX_EXPECTED_PACKET_FILLS },
            { RESOLUTION_CTRL_HIGH, BUDDY_DATA_SIZE_HIGH, (MAX_EXPECTED_PACKET_FILLS / 2) },
            { RESOLUTION_CTRL_SUPER, BUDDY_DATA_SIZE_SUPER, MAX_EXPECTED_PACKET_FILLS / 4 },
        };

        _chan_enable[BUDDY_CHAN_0] = 1; // only activate channel 0

        for (const auto &item : expectedResponse) {
            int res_ctrl = std::get<0>(item);
            int buddy_size = std::get<1>(item);
            int expectedCount = std::get<2>(item);

            reset_state(res_ctrl, buddy_size);
            reset_codec();

            // initials
            for (int i = 0; i <= (expectedCount - 1); i++) {
                test_buffer[BUDDY_APP_INDIC_OFFSET] =  expectedCount;
                REQUIRE( decode((uint8_t *) &test_buffer, &packet) == CODEC_STATUS_CONTINUE );

                if (res_ctrl == RESOLUTION_CTRL_LOW) {
                    REQUIRE( codec_byte_offset == (i + 1) );
                } else if (res_ctrl == RESOLUTION_CTRL_HIGH) {
                    REQUIRE( codec_byte_offset == ((i + 1) * 2) );
                } else if (res_ctrl == RESOLUTION_CTRL_SUPER) {
                    REQUIRE( codec_byte_offset == ((i + 1) * 4) );
                }
            }

            // final
            REQUIRE( decode((uint8_t *) &test_buffer, &packet) == CODEC_STATUS_FULL );
            REQUIRE( codec_byte_offset == 0 );
        }
    }

    SECTION( "buddy decode correctly rejects invalid resolution modes", "" ) {
        _resolution_mode = RESOLUTION_CTRL_END_MARKER;

        REQUIRE( decode((uint8_t *) &test_buffer, &packet) == CODEC_STATUS_ERROR );
    }
}

/*
TEST_CASE( "", "" ) {
    SECTION( "", "" ) {
        
    }
}
*/