# directory creation
add_custom_command(OUTPUT mkdir_doxygen_html
   COMMAND mkdir -p ${CMAKE_BINARY_DIR}/docs/firmware_html ${CMAKE_BINARY_DIR}/docs/libbuddy_html 
)

# generate doxygen for firmware
add_custom_target(
   build_doxygen_firmware ALL
   COMMAND ./build_doxygen.sh firmware ${CMAKE_BINARY_DIR}/docs/firmware_html
   WORKING_DIRECTORY ../../docs
   DEPENDS mkdir_doxygen_html
)

# generate doxygen for libbuddy library
add_custom_target(
   build_doxygen_lib ALL
   COMMAND ./build_doxygen.sh libbuddy ${CMAKE_BINARY_DIR}/docs/libbuddy_html
   WORKING_DIRECTORY ../../docs
   DEPENDS mkdir_doxygen_html
)