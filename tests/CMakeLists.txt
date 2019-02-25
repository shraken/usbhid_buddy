project( testLibBuddy )
cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 11)

# header files
set (MAIN_INCLUDE ${PROJECT_SOURCE_DIR}) 
set (LIBBUDDY_COMMON_INCLUDE ${PROJECT_SOURCE_DIR}/../lib/libbuddy/common)
set (LIBBUDDY_HOST_INCLUDE ${PROJECT_SOURCE_DIR}/../lib/libbuddy/host)
set (DUMMY_HIDAPI_INCLUDE ${PROJECT_SOURCE_DIR}/dummy_hidapi) 
set (BUDDY_INCLUDE_DIRS 
        ${MAIN_INCLUDE} 
        ${LIBBUDDY_COMMON_INCLUDE}
        ${LIBBUDDY_HOST_INCLUDE}
        ${DUMMY_HIDAPI_INCLUDE}
)

# source files
set( LIBBUDDY_SOURCES
    ${PROJECT_SOURCE_DIR}/../lib/libbuddy/common/codec.c
    ${PROJECT_SOURCE_DIR}/../lib/libbuddy/common/support.c
    ${PROJECT_SOURCE_DIR}/../lib/libbuddy/host/buddy.c
)

set( HIDAPI_SOURCES ${PROJECT_SOURCE_DIR}/dummy_hidapi/hid.c )

set (BUDDY_SOURCES ${LIBBUDDY_SOURCES} ${HIDAPI_SOURCES})

include_directories(${BUDDY_INCLUDE_DIRS})
add_executable (testLibBuddy testLibBuddy.cpp ${BUDDY_SOURCES})

target_link_libraries(testLibBuddy ${EXTRA_LIBS})
target_link_libraries(testLibBuddy ${EXTRA_LIBS} --coverage)

target_compile_options( testLibBuddy PRIVATE -fprofile-arcs -ftest-coverage -O0 -g --coverage)
set_target_properties( testLibBuddy PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage -O0 -g --coverage" )

# generate coverage report
add_custom_command(OUTPUT html
   COMMAND mkdir -p html
)

add_custom_target(
   build_coverage_report ALL
   COMMAND gcovr -r ../../ -e /docker/tests --html --html-details -o html/coverage.html
   DEPENDS html
)