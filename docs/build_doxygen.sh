#!/bin/sh
## Meant to be called by CMake project script to adjust output directory
## to build directory instead of project path. 

if [ "$#" -ne 2 ]; then
    echo "Incorrect number of arguments." 
    echo "Usage: ./build_doxygen.sh <firmware/libbuddy> <path to doxygen conf>"
fi

if [ $1 = "firmware" ]; then
    echo "Building doxygen for firmware"
    ( cat doxygen_firmware.conf ; echo "HTML_OUTPUT=$2" ) | doxygen -
elif [ $1 = "libbuddy" ]; then
    echo "Building doxygen for libbuddy"
    ( cat doxygen_libbuddy.conf ; echo "HTML_OUTPUT=$2" ) | doxygen -
else
    echo "Unknown doxygen output format requested"
fi