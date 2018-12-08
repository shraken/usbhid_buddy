[![Build Status](https://travis-ci.org/shraken/usbhid_buddy.svg?branch=master)](https://travis-ci.org/shraken/usbhid_buddy)

[![Coverage Status](https://coveralls.io/repos/github/shraken/usbhid_buddy/badge.svg)](https://coveralls.io/github/shraken/usbhid_buddy)

# Buddy DAQ

Buddy is a free, open source, and low-cost data acquisition (DAQ) instrument.  It has 8 configurable
IO that can be configured for ADC, DAC, PWM, or counter operation.  The device uses USB HID with a 
custom descriptor for communication with the hidapi library on the host system.  The device can be 
configured as either a host polled or asynchronous update mode for maximum flexibility.  

## Firmware

The firmware is written in C for the Silicon Labs C8051F380/EFM8UB2 chip.  

The open-source gboot bootloader is used.  Support is provided for the Windows, mac OS,
and Linux platforms.

## Hardware

The hardware is developed using the KiCad EDA tool.  The manafacturing, source, and schematic
files are provided in the hw/ directory.

The pin mapping for the standard Bus Pirate 10-pin harness is provided below:

| Color         | Mapping   | Pin #  |
| ------------- |:---------:|:------:|
| Brown         | IN7       |     2  |
| Red           | GND       |     1  |
| Orange        | IN6       |     4  |
| Yellow        | IN5       |     3  |
| Green         | IN0       |     6  |
| Blue          | IN4       |     5  |
| Purple        | IN2       |     8  |
| Grey          | IN1       |     7  |
| White         | 3.3V      |    10  |
| Black         | IN3       |     9  |

## Software

The host software uses the free and open source hidapi library for driverless
operation.  The examples provided build with the hidapi libray itself and
are tested on Linux, Windows, and mac OS systems.

For building the host examples, the hidapi submodule must be cloned with the
following command:

`git submodule update --init`

or when cloning the buddy repo with:

`git clone --recursive https://github.com/shraken/usbhid_buddy.git`

## Tests

The `tests` directory contains the unit tests.  The [Catch2](https://github.com/catchorg/Catch2) test
framework is used for all unit tests.  To manually build and run the unit test, run the following:

```shell
cd tests/
mkdir -p build
cd build
cmake ..
make
./testLibBuddy
```

## Code Coverage

The build server runs gcov/lcov to generate the coverage figure.  You can collect these results manually
by running the following (from the build directory in `tests`):

```shell
lcov -c -d CMakeFiles -out cov.info
genhtml cov.info -o out
open out/index.html
```