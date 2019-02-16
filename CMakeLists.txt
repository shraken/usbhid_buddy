cmake_minimum_required(VERSION 2.8)

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

# unit tests
add_subdirectory(tests)

# generate doxygen
# generate markdown doc
# generate coverage report