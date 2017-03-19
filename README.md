# Buddy DAQ

Buddy is a free, open source, and low-cost data acquisition (DAQ) instrument.  It has 8 configurable
IO that can be configured for ADC or DAC operation.  The device uses USB HID with a custom descriptor
for communication with the hidapi library on the host system.  The device can be configured as either
a host polled or asynchronous update mode for maximum flexibility.  

## Firmware

The firmware is written in C for the Silicon Labs C8051F380/EFM8UB2 chip.  

No bootloader solution exists currently but support is planned for
a future release.  

## Hardware

The hardware is developed using the KiCad EDA tool.  The manafacturing, source, and schematic
files are provided in the hw/ directory.

The pin mapping for the standard Bus Pirate 10-pin harness is provided below:

| Color         | Mapping   | Pin #  |
| ------------- |:---------:|:------:|
| Brown         | IN6       |     2  |
| Red           | GND       |     1  |
| Orange        | IN7       |     4  |
| Yellow        | IN5       |     3  |
| Green         | IN0       |     6  |
| Blue          | IN4       |     5  |
| Purple        | IN1       |     8  |
| Grey          | IN3       |     7  |
| White         | 3.3V      |    10  |
| Black         | IN2       |     9  |

## Software

The host software uses the free and open source hidapi library for driverless
operation.  The examples provided build with the hidapi libray itself and
are tested on Linux, Windows, and macOS systems.

For building the host examples, the hidapi submodule must be cloned with the
following command:

`git submodule update --init`

or when cloning the buddy repo with:

`git clone --recursive https://github.com/shraken/usbhid_buddy.git`