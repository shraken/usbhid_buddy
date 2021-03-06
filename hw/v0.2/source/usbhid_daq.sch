EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:microchip-ic
LIBS:usb
LIBS:ldo
LIBS:tactile
LIBS:leds
LIBS:reference
LIBS:switching_charge_pump
LIBS:dac_spi
LIBS:silabs
LIBS:esd_protection
LIBS:digital_switches
LIBS:open-project
LIBS:usbhid_daq-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 3
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 4150 2850 1350 1800
U 56F4B8A3
F0 "mcu" 60
F1 "mcu.sch" 60
F2 "ADC0_IN" I R 5500 3650 60 
F3 "UART_TX" O L 4150 4300 60 
F4 "UART_RX" I L 4150 4400 60 
F5 "SPI_SCLK" O R 5500 2950 60 
F6 "SPI_MOSI" O R 5500 3050 60 
F7 "SPI_CS" O R 5500 3250 60 
F8 "MCU_LDAC" O R 5500 3400 60 
F9 "ADC1_IN" I R 5500 3750 60 
F10 "ADC2_IN" I R 5500 3850 60 
F11 "ADC3_IN" I R 5500 3950 60 
F12 "ADC4_IN" I R 5500 4050 60 
F13 "ADC5_IN" I R 5500 4150 60 
F14 "ADC6_IN" I R 5500 4250 60 
F15 "ADC7_IN" I R 5500 4350 60 
$EndSheet
$Sheet
S 6350 2850 1250 1850
U 56F4BB54
F0 "periph" 60
F1 "periph.sch" 60
F2 "ADC0_IN" I L 6350 3650 60 
F3 "ADC1_IN" I L 6350 3750 60 
F4 "ADC2_IN" I L 6350 3850 60 
F5 "ADC3_IN" I L 6350 3950 60 
F6 "ADC4_IN" I L 6350 4050 60 
F7 "ADC5_IN" I L 6350 4150 60 
F8 "ADC6_IN" I L 6350 4250 60 
F9 "ADC7_IN" I L 6350 4350 60 
F10 "SPI_SCLK" I L 6350 2950 60 
F11 "SPI_SIMO" I L 6350 3050 60 
F12 "SPI_SOMI" O L 6350 3150 60 
F13 "SPI_CS" I L 6350 3250 60 
F14 "MCU_LDAC" I L 6350 3400 60 
$EndSheet
Wire Wire Line
	6350 3650 5500 3650
Wire Wire Line
	5500 3400 6350 3400
Wire Wire Line
	5500 3750 6350 3750
Wire Wire Line
	5500 3850 6350 3850
Wire Wire Line
	5500 3950 6350 3950
Wire Wire Line
	6350 4050 5500 4050
Wire Wire Line
	5500 4150 6350 4150
Wire Wire Line
	6350 4250 5500 4250
Wire Wire Line
	5500 4350 6350 4350
Wire Wire Line
	5500 3050 6350 3050
Wire Wire Line
	6350 2950 5500 2950
Wire Wire Line
	5500 3250 6350 3250
$Comp
L R R11
U 1 1 56F7D28A
P 5650 2450
F 0 "R11" V 5730 2450 50  0000 C CNN
F 1 "0R_NM" V 5650 2450 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 5580 2450 50  0001 C CNN
F 3 "" H 5650 2450 50  0000 C CNN
	1    5650 2450
	1    0    0    -1  
$EndComp
$Comp
L R R10
U 1 1 56F7D3AD
P 5900 2450
F 0 "R10" V 5980 2450 50  0000 C CNN
F 1 "10k" V 5900 2450 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 5830 2450 50  0001 C CNN
F 3 "" H 5900 2450 50  0000 C CNN
	1    5900 2450
	1    0    0    -1  
$EndComp
$Comp
L R R7
U 1 1 56F7D546
P 6150 2450
F 0 "R7" V 6230 2450 50  0000 C CNN
F 1 "10k" V 6150 2450 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 6080 2450 50  0001 C CNN
F 3 "" H 6150 2450 50  0000 C CNN
	1    6150 2450
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR01
U 1 1 56F7D66F
P 5650 2100
F 0 "#PWR01" H 5650 1850 50  0001 C CNN
F 1 "GND" H 5650 1950 50  0000 C CNN
F 2 "" H 5650 2100 50  0000 C CNN
F 3 "" H 5650 2100 50  0000 C CNN
	1    5650 2100
	-1   0    0    1   
$EndComp
Wire Wire Line
	5650 2300 5650 2100
Wire Wire Line
	5650 2600 5650 3400
Connection ~ 5650 3400
Wire Wire Line
	5900 2600 5900 2950
Connection ~ 5900 2950
Wire Wire Line
	6150 2600 6150 3050
Connection ~ 6150 3050
$Comp
L +3.3V #PWR02
U 1 1 56F7F240
P 6150 1750
F 0 "#PWR02" H 6150 1600 50  0001 C CNN
F 1 "+3.3V" H 6150 1890 50  0000 C CNN
F 2 "" H 6150 1750 50  0000 C CNN
F 3 "" H 6150 1750 50  0000 C CNN
	1    6150 1750
	1    0    0    -1  
$EndComp
Wire Wire Line
	6150 2300 6150 1750
Wire Wire Line
	5900 2300 5900 2150
Wire Wire Line
	5900 2150 6150 2150
Connection ~ 6150 2150
$EndSCHEMATC
