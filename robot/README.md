# Buddy Test Platform

This repo contains the manufacturing automated test system for the Buddy USB HID DAQ hardware
device.  

The automated test board mates with a Teensy 3.6 microcontroller for measurement and
stimulus of the "Buddy" DAQ under test.
 - `teensy/` - teensy Arduino firmware for the DUT test software
 - `sw/` - host based robot framework source files to be run by tester
 - `hw/` - PCB schematic, layout, and datasheet files

# Requirements

Install the [ADC](https://github.com/pedvide/ADC) library for the Arduino software.  

# Building

There is no automated build procedure.  

1. Open Arduino IDE 

Modify the `RingBuffer.h` file and set the define `RING_BUFFER_DEFAULT_BUFFER_SIZE` to 4096.  This
requires modification of the `RingBuffer.h` file provided in the `ADC` library installed above.  

# Usage

on Windows

```shell
cd sw/
./run_tests.bat
```

on Linux and Mac OS

```shell
cd sw/
./run_tests.sh
```