#define CATCH_CONFIG_MAIN

#include "catch.hpp"

extern "C" {
#include "support.h"
#include "utility.h"
#include "usbhid_buddy.h"
}

TEST_CASE( "test case #1", "" ) {
    REQUIRE( swap_uint16(0x1234) == 0x3412 );
}

TEST_CASE( "test case #2", "" ) {
    REQUIRE( 1 == 1 );
}