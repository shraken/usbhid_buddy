EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:switches
LIBS:relays
LIBS:motors
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
LIBS:multiplexers
LIBS:digital_switches
LIBS:tristate
LIBS:samtec
LIBS:usbhid_testfixture-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 3
Title "connectors"
Date "2018-04-22"
Rev "v0.2"
Comp "Wiggle Labs"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Conn_02x05_Odd_Even J4
U 1 1 5A269FD2
P 9500 2800
F 0 "J4" H 9550 3100 50  0000 C CNN
F 1 "Conn_02x05_Odd_Even" H 9550 2500 50  0000 C CNN
F 2 "Socket_Strips:Socket_Strip_Straight_2x05_Pitch2.54mm" H 9500 2800 50  0001 C CNN
F 3 "" H 9500 2800 50  0001 C CNN
	1    9500 2800
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR01
U 1 1 5A26A1C7
P 2350 3500
F 0 "#PWR01" H 2350 3250 50  0001 C CNN
F 1 "GND" H 2350 3350 50  0000 C CNN
F 2 "" H 2350 3500 50  0001 C CNN
F 3 "" H 2350 3500 50  0001 C CNN
	1    2350 3500
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR02
U 1 1 5A26B17A
P 3550 1850
F 0 "#PWR02" H 3550 1700 50  0001 C CNN
F 1 "+3.3V" H 3550 1990 50  0000 C CNN
F 2 "" H 3550 1850 50  0001 C CNN
F 3 "" H 3550 1850 50  0001 C CNN
	1    3550 1850
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR03
U 1 1 5A26B288
P 5600 900
F 0 "#PWR03" H 5600 650 50  0001 C CNN
F 1 "GND" H 5600 750 50  0000 C CNN
F 2 "" H 5600 900 50  0001 C CNN
F 3 "" H 5600 900 50  0001 C CNN
	1    5600 900 
	-1   0    0    1   
$EndComp
$Comp
L VCC #PWR04
U 1 1 5A26B3BF
P 5300 900
F 0 "#PWR04" H 5300 750 50  0001 C CNN
F 1 "VCC" H 5300 1050 50  0000 C CNN
F 2 "" H 5300 900 50  0001 C CNN
F 3 "" H 5300 900 50  0001 C CNN
	1    5300 900 
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR05
U 1 1 5A26C5DD
P 9150 2450
F 0 "#PWR05" H 9150 2200 50  0001 C CNN
F 1 "GND" H 9150 2300 50  0000 C CNN
F 2 "" H 9150 2450 50  0001 C CNN
F 3 "" H 9150 2450 50  0001 C CNN
	1    9150 2450
	-1   0    0    1   
$EndComp
Text HLabel 10400 2800 2    60   BiDi ~ 0
FIO_0
Text HLabel 10400 2700 2    60   BiDi ~ 0
FIO_6
Text HLabel 10400 2600 2    60   BiDi ~ 0
FIO_7
Text HLabel 10400 2900 2    60   BiDi ~ 0
FIO_2
Text HLabel 8850 2700 0    60   BiDi ~ 0
FIO_5
Text HLabel 8850 2800 0    60   BiDi ~ 0
FIO_4
Text HLabel 8850 2900 0    60   BiDi ~ 0
FIO_1
Text HLabel 8850 3000 0    60   BiDi ~ 0
FIO_3
Text Label 8950 2700 0    60   ~ 0
FIO_5
Text Label 8950 2800 0    60   ~ 0
FIO_4
Text Label 8950 2900 0    60   ~ 0
FIO_1
Text Label 8950 3000 0    60   ~ 0
FIO_3
Text Label 9950 2900 0    60   ~ 0
FIO_2
Text Label 9950 2800 0    60   ~ 0
FIO_0
Text Label 9950 2700 0    60   ~ 0
FIO_6
Text Label 9950 2600 0    60   ~ 0
FIO_7
$Comp
L BTE-020-01-F-D P1
U 1 1 5A2A7320
P 3700 5600
F 0 "P1" H 3650 6850 60  0000 C CNN
F 1 "BTE-020-01-F-D" H 3700 4550 60  0000 C CNN
F 2 "samtec:BTE-020-01-F-D-A" H 3950 5900 60  0001 C CNN
F 3 "" H 3950 5900 60  0000 C CNN
	1    3700 5600
	1    0    0    -1  
$EndComp
Text Label 2900 4550 0    60   ~ 0
FIO_0
Text Label 2900 4750 0    60   ~ 0
FIO_1
Text Label 2900 4950 0    60   ~ 0
FIO_2
Text Label 2900 5150 0    60   ~ 0
FIO_3
Text Label 4400 4550 0    60   ~ 0
FIO_7
Text Label 4400 4750 0    60   ~ 0
FIO_6
Text Label 4400 4950 0    60   ~ 0
FIO_5
Text Label 4400 5150 0    60   ~ 0
FIO_4
$Comp
L GND #PWR06
U 1 1 5A2A7959
P 2300 5550
F 0 "#PWR06" H 2300 5300 50  0001 C CNN
F 1 "GND" H 2300 5400 50  0000 C CNN
F 2 "" H 2300 5550 50  0001 C CNN
F 3 "" H 2300 5550 50  0001 C CNN
	1    2300 5550
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR07
U 1 1 5A2A7B17
P 5250 5550
F 0 "#PWR07" H 5250 5300 50  0001 C CNN
F 1 "GND" H 5250 5400 50  0000 C CNN
F 2 "" H 5250 5550 50  0001 C CNN
F 3 "" H 5250 5550 50  0001 C CNN
	1    5250 5550
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR08
U 1 1 5A2A7DB3
P 3100 6150
F 0 "#PWR08" H 3100 5900 50  0001 C CNN
F 1 "GND" H 3100 6000 50  0000 C CNN
F 2 "" H 3100 6150 50  0001 C CNN
F 3 "" H 3100 6150 50  0001 C CNN
	1    3100 6150
	0    1    1    0   
$EndComp
$Comp
L GND #PWR09
U 1 1 5A2A7E7A
P 4250 6150
F 0 "#PWR09" H 4250 5900 50  0001 C CNN
F 1 "GND" H 4250 6000 50  0000 C CNN
F 2 "" H 4250 6150 50  0001 C CNN
F 3 "" H 4250 6150 50  0001 C CNN
	1    4250 6150
	0    -1   -1   0   
$EndComp
Text HLabel 2800 3300 2    60   BiDi ~ 0
TEENSY_0
Text HLabel 2800 3200 2    60   BiDi ~ 0
TEENSY_1
Text HLabel 2800 3100 2    60   BiDi ~ 0
TEENSY_2
Text HLabel 2800 3000 2    60   BiDi ~ 0
TEENSY_3
Text HLabel 2800 2900 2    60   BiDi ~ 0
TEENSY_4
Text HLabel 2800 2800 2    60   BiDi ~ 0
TEENSY_5
Text HLabel 2800 2700 2    60   BiDi ~ 0
TEENSY_6
Text HLabel 2800 2600 2    60   BiDi ~ 0
TEENSY_7
Text HLabel 2800 2500 2    60   BiDi ~ 0
TEENSY_8
Text HLabel 2800 2400 2    60   BiDi ~ 0
TEENSY_9
Text HLabel 2800 2300 2    60   BiDi ~ 0
TEENSY_10
Text HLabel 2800 2200 2    60   BiDi ~ 0
TEENSY_11
Text HLabel 2800 2100 2    60   BiDi ~ 0
TEENSY_12
Text HLabel 2800 1900 2    60   BiDi ~ 0
TEENSY_24
Text HLabel 2800 1800 2    60   BiDi ~ 0
TEENSY_25
Text HLabel 2800 1700 2    60   BiDi ~ 0
TEENSY_26
Text HLabel 2800 1600 2    60   BiDi ~ 0
TEENSY_27
Text HLabel 2800 1500 2    60   BiDi ~ 0
TEENSY_28
Text HLabel 2800 1400 2    60   BiDi ~ 0
TEENSY_29
Text HLabel 2800 1300 2    60   BiDi ~ 0
TEENSY_30
Text HLabel 2800 1200 2    60   BiDi ~ 0
TEENSY_31
Text HLabel 2800 1100 2    60   BiDi ~ 0
TEENSY_32
Text HLabel 5000 1400 2    60   BiDi ~ 0
TEENSY_23
Text HLabel 5000 1500 2    60   BiDi ~ 0
TEENSY_22
Text HLabel 5000 1600 2    60   BiDi ~ 0
TEENSY_21
Text HLabel 5000 1700 2    60   BiDi ~ 0
TEENSY_20
Text HLabel 5000 1800 2    60   BiDi ~ 0
TEENSY_19
Text HLabel 5000 1900 2    60   BiDi ~ 0
TEENSY_18
Text HLabel 5000 2000 2    60   BiDi ~ 0
TEENSY_17
Text HLabel 5000 2100 2    60   BiDi ~ 0
TEENSY_16
Text HLabel 5000 2200 2    60   BiDi ~ 0
TEENSY_15
Text HLabel 5000 2300 2    60   BiDi ~ 0
TEENSY_14
Text HLabel 5000 2400 2    60   BiDi ~ 0
TEENSY_13
Text HLabel 5000 2600 2    60   Output ~ 0
TEENSY_DAC1
Text HLabel 5000 2700 2    60   Output ~ 0
TEENSY_DAC0
Text HLabel 5000 2800 2    60   BiDi ~ 0
TEENSY_39
Text HLabel 5000 2900 2    60   BiDi ~ 0
TEENSY_38
Text HLabel 5000 3000 2    60   BiDi ~ 0
TEENSY_37
Text HLabel 5000 3100 2    60   BiDi ~ 0
TEENSY_36
Text HLabel 5000 3200 2    60   BiDi ~ 0
TEENSY_35
Text HLabel 5000 3300 2    60   BiDi ~ 0
TEENSY_34
Text HLabel 5000 3400 2    60   BiDi ~ 0
TEENSY_33
$Comp
L GND #PWR010
U 1 1 5ADD7269
P 5900 2600
F 0 "#PWR010" H 5900 2350 50  0001 C CNN
F 1 "GND" H 5900 2450 50  0000 C CNN
F 2 "" H 5900 2600 50  0001 C CNN
F 3 "" H 5900 2600 50  0001 C CNN
	1    5900 2600
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR011
U 1 1 5ADD76F8
P 5850 900
F 0 "#PWR011" H 5850 750 50  0001 C CNN
F 1 "+3.3V" H 5850 1040 50  0000 C CNN
F 2 "" H 5850 900 50  0001 C CNN
F 3 "" H 5850 900 50  0001 C CNN
	1    5850 900 
	1    0    0    -1  
$EndComp
$Comp
L R R6
U 1 1 5AD233D1
P 6550 5550
F 0 "R6" V 6630 5550 50  0000 C CNN
F 1 "1k" V 6550 5550 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 6480 5550 50  0001 C CNN
F 3 "" H 6550 5550 50  0001 C CNN
	1    6550 5550
	1    0    0    -1  
$EndComp
$Comp
L LED D1
U 1 1 5AD232CE
P 6550 5100
F 0 "D1" H 6550 5200 50  0000 C CNN
F 1 "LED" H 6550 5000 50  0000 C CNN
F 2 "LEDs:LED_0603_HandSoldering" H 6550 5100 50  0001 C CNN
F 3 "" H 6550 5100 50  0001 C CNN
	1    6550 5100
	0    1    1    0   
$EndComp
$Comp
L +3.3V #PWR012
U 1 1 5ADD88AE
P 6550 6000
F 0 "#PWR012" H 6550 5850 50  0001 C CNN
F 1 "+3.3V" H 6550 6140 50  0000 C CNN
F 2 "" H 6550 6000 50  0001 C CNN
F 3 "" H 6550 6000 50  0001 C CNN
	1    6550 6000
	-1   0    0    1   
$EndComp
$Comp
L LED D2
U 1 1 5ADD89EF
P 7000 5100
F 0 "D2" H 7000 5200 50  0000 C CNN
F 1 "LED" H 7000 5000 50  0000 C CNN
F 2 "LEDs:LED_0603_HandSoldering" H 7000 5100 50  0001 C CNN
F 3 "" H 7000 5100 50  0001 C CNN
	1    7000 5100
	0    1    1    0   
$EndComp
$Comp
L R R7
U 1 1 5ADD8A34
P 7000 5550
F 0 "R7" V 7080 5550 50  0000 C CNN
F 1 "1k" V 7000 5550 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 6930 5550 50  0001 C CNN
F 3 "" H 7000 5550 50  0001 C CNN
	1    7000 5550
	1    0    0    -1  
$EndComp
Wire Wire Line
	1950 2100 2800 2100
Wire Wire Line
	1950 2200 2800 2200
Wire Wire Line
	1950 2300 2800 2300
Wire Wire Line
	1950 2400 2800 2400
Wire Wire Line
	1950 2500 2800 2500
Wire Wire Line
	1950 2600 2800 2600
Wire Wire Line
	1950 2700 2800 2700
Wire Wire Line
	1950 2800 2800 2800
Wire Wire Line
	1950 2900 2800 2900
Wire Wire Line
	1950 3000 2800 3000
Wire Wire Line
	1950 3100 2800 3100
Wire Wire Line
	1950 3200 2800 3200
Wire Wire Line
	1950 3300 2800 3300
Wire Wire Line
	4200 2400 5000 2400
Wire Wire Line
	4200 2500 5900 2500
Wire Wire Line
	4200 2600 5000 2600
Wire Wire Line
	4200 2700 5000 2700
Wire Wire Line
	4200 2800 5000 2800
Wire Wire Line
	4200 2900 5000 2900
Wire Wire Line
	4200 3000 5000 3000
Wire Wire Line
	4200 3100 5000 3100
Wire Wire Line
	4200 3200 5000 3200
Wire Wire Line
	4200 3300 5000 3300
Wire Wire Line
	4200 3400 5000 3400
Wire Wire Line
	9300 2600 9150 2600
Wire Wire Line
	9150 2600 9150 2450
Wire Wire Line
	9300 2700 8850 2700
Wire Wire Line
	9300 2800 8850 2800
Wire Wire Line
	9300 2900 8850 2900
Wire Wire Line
	9300 3000 8850 3000
Wire Wire Line
	9800 2600 10400 2600
Wire Wire Line
	9800 2700 10400 2700
Wire Wire Line
	9800 2800 10400 2800
Wire Wire Line
	9800 2900 10400 2900
Wire Wire Line
	3400 4550 2600 4550
Wire Wire Line
	3400 4750 2600 4750
Wire Wire Line
	3400 4950 2600 4950
Wire Wire Line
	3400 5150 2600 5150
Wire Wire Line
	3950 4550 4950 4550
Wire Wire Line
	3950 4750 4950 4750
Wire Wire Line
	3950 4950 4950 4950
Wire Wire Line
	3950 5150 4950 5150
Wire Wire Line
	2300 5250 3400 5250
Wire Wire Line
	2300 4650 2300 5550
Wire Wire Line
	3400 5050 2300 5050
Connection ~ 2300 5250
Wire Wire Line
	3400 4850 2300 4850
Connection ~ 2300 5050
Wire Wire Line
	3400 4650 2300 4650
Connection ~ 2300 4850
Wire Wire Line
	5250 5250 3950 5250
Wire Wire Line
	5250 4650 5250 5550
Wire Wire Line
	3950 5050 5250 5050
Connection ~ 5250 5250
Wire Wire Line
	3950 4850 5250 4850
Connection ~ 5250 5050
Wire Wire Line
	3950 4650 5250 4650
Connection ~ 5250 4850
Wire Wire Line
	3100 6150 3400 6150
Wire Wire Line
	3400 6250 3300 6250
Wire Wire Line
	3300 6250 3300 6150
Connection ~ 3300 6150
Wire Wire Line
	3950 6150 4250 6150
Wire Wire Line
	3950 6250 4100 6250
Wire Wire Line
	4100 6250 4100 6150
Connection ~ 4100 6150
Wire Wire Line
	4200 2300 5000 2300
Wire Wire Line
	4200 2200 5000 2200
Wire Wire Line
	4200 2100 5000 2100
Wire Wire Line
	4200 2000 5000 2000
Wire Wire Line
	4200 1900 5000 1900
Wire Wire Line
	4200 1800 5000 1800
Wire Wire Line
	4200 1700 5000 1700
Wire Wire Line
	4200 1600 5000 1600
Wire Wire Line
	4200 1500 5000 1500
Wire Wire Line
	4200 1400 5000 1400
Wire Wire Line
	4200 1300 5850 1300
Wire Wire Line
	4200 1200 5600 1200
Wire Wire Line
	4200 1100 5300 1100
Wire Wire Line
	1950 2000 3550 2000
Wire Wire Line
	1950 1900 2800 1900
Wire Wire Line
	1950 1800 2800 1800
Wire Wire Line
	1950 1700 2800 1700
Wire Wire Line
	1950 1600 2800 1600
Wire Wire Line
	1950 1500 2800 1500
Wire Wire Line
	1950 1400 2800 1400
Wire Wire Line
	1950 1300 2800 1300
Wire Wire Line
	1950 1200 2800 1200
Wire Wire Line
	1950 1100 2800 1100
Wire Wire Line
	1950 3400 2350 3400
Wire Wire Line
	2350 3400 2350 3500
Wire Wire Line
	3550 2000 3550 1850
Wire Wire Line
	5900 2500 5900 2600
Wire Wire Line
	5850 1300 5850 900 
Wire Wire Line
	5600 1200 5600 900 
Wire Wire Line
	5300 1100 5300 900 
Wire Wire Line
	6550 6000 6550 5700
Wire Wire Line
	7000 5700 7000 5850
Wire Wire Line
	7000 5850 6550 5850
Connection ~ 6550 5850
Wire Wire Line
	6550 5400 6550 5250
Wire Wire Line
	7000 5400 7000 5250
Wire Wire Line
	7000 4950 7000 4800
Wire Wire Line
	7000 4800 7900 4800
Wire Wire Line
	6550 4950 6550 4700
Wire Wire Line
	6550 4700 7900 4700
Text Label 7200 4800 0    60   ~ 0
TEENSY_38
Text Label 7200 4700 0    60   ~ 0
TEENSY_39
$Comp
L VCC #PWR013
U 1 1 5AE1ABB9
P 10150 3200
F 0 "#PWR013" H 10150 3050 50  0001 C CNN
F 1 "VCC" H 10150 3350 50  0000 C CNN
F 2 "" H 10150 3200 50  0001 C CNN
F 3 "" H 10150 3200 50  0001 C CNN
	1    10150 3200
	-1   0    0    1   
$EndComp
Wire Wire Line
	9800 3000 10150 3000
Wire Wire Line
	10150 3000 10150 3200
$Comp
L Conn_01x24 J1
U 1 1 5AE1AA24
P 1750 2300
F 0 "J1" H 1750 3500 50  0000 C CNN
F 1 "Conn_01x24" H 1750 1000 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x24_Pitch2.54mm" H 1750 2300 50  0001 C CNN
F 3 "" H 1750 2300 50  0001 C CNN
	1    1750 2300
	-1   0    0    1   
$EndComp
$Comp
L Conn_01x24 J2
U 1 1 5AE1AA9A
P 4000 2300
F 0 "J2" H 4000 3500 50  0000 C CNN
F 1 "Conn_01x24" H 4000 1000 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x24_Pitch2.54mm" H 4000 2300 50  0001 C CNN
F 3 "" H 4000 2300 50  0001 C CNN
	1    4000 2300
	-1   0    0    1   
$EndComp
$EndSCHEMATC
