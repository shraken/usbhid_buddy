cmake_minimum_required(VERSION 2.8)

set(basedir ${CMAKE_CURRENT_SOURCE_DIR})
set(libdir ${basedir}/lib/)
set(libbuddydir ${basedir}/lib/libbuddy)

#message(STATUS "basedir = ${basedir}")
#message(STATUS "libbuddydir = ${libbuddydir}")

# firmware
# firmware unit tests

# bootload firmware
add_custom_target(
   build_gboot ALL
   COMMAND make FLASH_SIZE=64 VARIANT=F38x
   WORKING_DIRECTORY ../bootloader/gboot
)

# bootload flash utility
add_subdirectory(bootloader/gflash)

# examples
add_subdirectory(examples/C/simple)

# libbuddy library
add_subdirectory(lib/libbuddy)

# libbuddy bindings
add_subdirectory(lib/libbuddy/bindings)

# unit tests
add_subdirectory(tests)

# generate libbuddy library doxygen
# generate firmware doxygen
# generate markdown doc
add_subdirectory(docs)