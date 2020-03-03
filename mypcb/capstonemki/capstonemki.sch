EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Device:R_Small_US R7
U 1 1 5E34EEE2
P 3350 6900
F 0 "R7" H 3418 6946 50  0000 L CNN
F 1 "10K" H 3418 6855 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 3350 6900 50  0001 C CNN
F 3 "~" H 3350 6900 50  0001 C CNN
	1    3350 6900
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 6800 3350 6800
$Comp
L power:GND #PWR011
U 1 1 5E352719
P 3350 7000
F 0 "#PWR011" H 3350 6750 50  0001 C CNN
F 1 "GND" H 3355 6827 50  0000 C CNN
F 2 "" H 3350 7000 50  0001 C CNN
F 3 "" H 3350 7000 50  0001 C CNN
	1    3350 7000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR05
U 1 1 5E35B029
P 1900 6800
F 0 "#PWR05" H 1900 6550 50  0001 C CNN
F 1 "GND" H 1905 6627 50  0000 C CNN
F 2 "" H 1900 6800 50  0001 C CNN
F 3 "" H 1900 6800 50  0001 C CNN
	1    1900 6800
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 7100 2000 7200
$Comp
L power:GND #PWR06
U 1 1 5E354C14
P 2000 7400
F 0 "#PWR06" H 2000 7150 50  0001 C CNN
F 1 "GND" H 2005 7227 50  0000 C CNN
F 2 "" H 2000 7400 50  0001 C CNN
F 3 "" H 2000 7400 50  0001 C CNN
	1    2000 7400
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small_US R4
U 1 1 5E354C0D
P 2000 7300
F 0 "R4" H 2068 7346 50  0000 L CNN
F 1 "3.3K" H 2068 7255 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 2000 7300 50  0001 C CNN
F 3 "~" H 2000 7300 50  0001 C CNN
	1    2000 7300
	1    0    0    -1  
$EndComp
$Comp
L Battery_Management:BQ24075 U1
U 1 1 5E34979F
P 2600 6700
F 0 "U1" H 2600 7481 50  0000 C CNN
F 1 "BQ24075" H 2600 7390 50  0000 C CNN
F 2 "Package_DFN_QFN:QFN-16-1EP_3x3mm_P0.5mm_EP1.7x1.7mm" H 2850 6150 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/bq24075.pdf" H 2800 6900 50  0001 C CNN
	1    2600 6700
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small_US R3
U 1 1 5E360820
P 1750 7100
F 0 "R3" H 1818 7146 50  0000 L CNN
F 1 "1.1K" H 1818 7055 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 1750 7100 50  0001 C CNN
F 3 "~" H 1750 7100 50  0001 C CNN
	1    1750 7100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR03
U 1 1 5E360827
P 1750 7200
F 0 "#PWR03" H 1750 6950 50  0001 C CNN
F 1 "GND" H 1755 7027 50  0000 C CNN
F 2 "" H 1750 7200 50  0001 C CNN
F 3 "" H 1750 7200 50  0001 C CNN
	1    1750 7200
	1    0    0    -1  
$EndComp
Wire Wire Line
	1750 7000 2000 7000
$Comp
L Device:R_Small_US R1
U 1 1 5E3621C9
P 1450 6600
F 0 "R1" H 1518 6646 50  0000 L CNN
F 1 "100K" H 1518 6555 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 1450 6600 50  0001 C CNN
F 3 "~" H 1450 6600 50  0001 C CNN
	1    1450 6600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR01
U 1 1 5E3621CF
P 1450 6700
F 0 "#PWR01" H 1450 6450 50  0001 C CNN
F 1 "GND" H 1455 6527 50  0000 C CNN
F 2 "" H 1450 6700 50  0001 C CNN
F 3 "" H 1450 6700 50  0001 C CNN
	1    1450 6700
	1    0    0    -1  
$EndComp
Wire Wire Line
	1450 6500 2000 6500
$Comp
L Connector_Generic:Conn_01x02 J2
U 1 1 5E364C9A
P 3800 4850
F 0 "J2" H 3880 4842 50  0000 L CNN
F 1 "Conn_01x02" H 3880 4751 50  0000 L CNN
F 2 "Connector_JST:JST_PH_B2B-PH-K_1x02_P2.00mm_Vertical" H 3800 4850 50  0001 C CNN
F 3 "~" H 3800 4850 50  0001 C CNN
	1    3800 4850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR012
U 1 1 5E36851C
P 3600 4950
F 0 "#PWR012" H 3600 4700 50  0001 C CNN
F 1 "GND" H 3605 4777 50  0000 C CNN
F 2 "" H 3600 4950 50  0001 C CNN
F 3 "" H 3600 4950 50  0001 C CNN
	1    3600 4950
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 6700 3200 6600
$Comp
L Device:R_US R8
U 1 1 5E3E1246
P 3850 6150
F 0 "R8" V 3645 6150 50  0000 C CNN
F 1 "180" V 3736 6150 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 3890 6140 50  0001 C CNN
F 3 "~" H 3850 6150 50  0001 C CNN
	1    3850 6150
	0    1    1    0   
$EndComp
Wire Wire Line
	3550 6150 3700 6150
$Comp
L Device:LED D1
U 1 1 5E3DE77F
P 3400 6150
F 0 "D1" H 3393 5895 50  0000 C CNN
F 1 "BLUE LED" H 3393 5986 50  0000 C CNN
F 2 "LED_SMD:LED_0603_1608Metric" H 3400 6150 50  0001 C CNN
F 3 "~" H 3400 6150 50  0001 C CNN
	1    3400 6150
	-1   0    0    1   
$EndComp
Wire Wire Line
	4000 6150 4500 6150
Wire Wire Line
	4500 6150 4500 7250
Wire Wire Line
	4500 7250 3200 7250
Wire Wire Line
	3200 7250 3200 7100
$Comp
L Device:R_US R9
U 1 1 5E3E7F78
P 3850 6500
F 0 "R9" V 3645 6500 50  0000 C CNN
F 1 "1K" V 3736 6500 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 3890 6490 50  0001 C CNN
F 3 "~" H 3850 6500 50  0001 C CNN
	1    3850 6500
	0    1    1    0   
$EndComp
Wire Wire Line
	3550 6500 3700 6500
Wire Wire Line
	4000 6500 4450 6500
Wire Wire Line
	4450 7200 3250 7200
Wire Wire Line
	3250 7200 3250 7000
Wire Wire Line
	3250 7000 3200 7000
$Comp
L power:GND #PWR09
U 1 1 5E3E8F29
P 2550 7300
F 0 "#PWR09" H 2550 7050 50  0001 C CNN
F 1 "GND" H 2555 7127 50  0000 C CNN
F 2 "" H 2550 7300 50  0001 C CNN
F 3 "" H 2550 7300 50  0001 C CNN
	1    2550 7300
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 7300 2550 7300
Connection ~ 2550 7300
Wire Wire Line
	2550 7300 2600 7300
$Comp
L Device:C C4
U 1 1 5E3EB1B8
P 4650 6300
F 0 "C4" H 4765 6346 50  0000 L CNN
F 1 "10UF" H 4765 6255 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 4688 6150 50  0001 C CNN
F 3 "~" H 4650 6300 50  0001 C CNN
	1    4650 6300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR013
U 1 1 5E3EC2B6
P 4650 6450
F 0 "#PWR013" H 4650 6200 50  0001 C CNN
F 1 "GND" H 4655 6277 50  0000 C CNN
F 2 "" H 4650 6450 50  0001 C CNN
F 3 "" H 4650 6450 50  0001 C CNN
	1    4650 6450
	1    0    0    -1  
$EndComp
Wire Wire Line
	4450 6500 4450 7200
Text GLabel 3300 4850 0    50   Input ~ 0
battery_in
Text GLabel 3200 6650 2    50   Input ~ 0
V_battery
$Comp
L Device:LED D2
U 1 1 5E3E7F7F
P 3400 6500
F 0 "D2" H 3393 6245 50  0000 C CNN
F 1 "RED LED" H 3393 6336 50  0000 C CNN
F 2 "LED_SMD:LED_0603_1608Metric" H 3400 6500 50  0001 C CNN
F 3 "~" H 3400 6500 50  0001 C CNN
	1    3400 6500
	-1   0    0    1   
$EndComp
NoConn ~ 2000 6600
$Comp
L Connector:USB_B_Micro J1
U 1 1 5E3F8122
P 1750 5050
F 0 "J1" H 1807 5517 50  0000 C CNN
F 1 "USB_B_Micro" H 1807 5426 50  0000 C CNN
F 2 "Connector_USB:USB_Micro-B_Molex-105017-0001" H 1900 5000 50  0001 C CNN
F 3 "~" H 1900 5000 50  0001 C CNN
	1    1750 5050
	1    0    0    -1  
$EndComp
NoConn ~ 2050 5050
NoConn ~ 2050 5150
NoConn ~ 2050 5250
NoConn ~ 1650 5450
$Comp
L power:GND #PWR08
U 1 1 5E3FBEE9
P 1750 5450
F 0 "#PWR08" H 1750 5200 50  0001 C CNN
F 1 "GND" H 1755 5277 50  0000 C CNN
F 2 "" H 1750 5450 50  0001 C CNN
F 3 "" H 1750 5450 50  0001 C CNN
	1    1750 5450
	1    0    0    -1  
$EndComp
Text GLabel 2350 4850 2    50   Input ~ 0
V_USB
Text GLabel 2600 6100 2    50   Input ~ 0
V_USB
$Comp
L Battery_Management:BQ27441-G1 U2
U 1 1 5E3FF08B
P 2100 2600
F 0 "U2" H 2100 3267 50  0000 C CNN
F 1 "BQ27441-G1" H 2100 3176 50  0000 C CNN
F 2 "Package_SON:Texas_S-PDSO-N12" H 2350 2050 50  0001 L CNN
F 3 "http://www.ti.com/lit/ds/symlink/bq27441-g1.pdf" H 2300 2800 50  0001 C CNN
	1    2100 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 3200 2050 3200
$Comp
L power:GND #PWR010
U 1 1 5E453F97
P 2050 3200
F 0 "#PWR010" H 2050 2950 50  0001 C CNN
F 1 "GND" H 2055 3027 50  0000 C CNN
F 2 "" H 2050 3200 50  0001 C CNN
F 3 "" H 2050 3200 50  0001 C CNN
	1    2050 3200
	1    0    0    -1  
$EndComp
Connection ~ 2050 3200
Wire Wire Line
	2050 3200 2100 3200
Wire Wire Line
	950  2800 1500 2800
$Comp
L Device:C C1
U 1 1 5E455A4A
P 950 2950
F 0 "C1" H 1065 2996 50  0000 L CNN
F 1 "1UF" H 1065 2905 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 988 2800 50  0001 C CNN
F 3 "~" H 950 2950 50  0001 C CNN
	1    950  2950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR02
U 1 1 5E45630F
P 950 3100
F 0 "#PWR02" H 950 2850 50  0001 C CNN
F 1 "GND" H 955 2927 50  0000 C CNN
F 2 "" H 950 3100 50  0001 C CNN
F 3 "" H 950 3100 50  0001 C CNN
	1    950  3100
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 5E458B51
P 3100 2800
F 0 "C3" H 3215 2846 50  0000 L CNN
F 1 "0.47UF" H 3215 2755 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3138 2650 50  0001 C CNN
F 3 "~" H 3100 2800 50  0001 C CNN
	1    3100 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 2950 3100 3200
Connection ~ 2100 3200
Text GLabel 4650 5850 2    50   Input ~ 0
Vout
Text GLabel 1500 2400 0    50   Output ~ 0
SDA
Text GLabel 1500 2500 0    50   Output ~ 0
SCL
Connection ~ 950  2800
Wire Wire Line
	950  2200 1500 2200
Wire Wire Line
	900  2200 950  2200
Connection ~ 950  2200
Wire Wire Line
	950  2200 950  2700
Text GLabel 900  2200 0    50   Input ~ 0
battery_in
$Comp
L Device:R_Small_US R2
U 1 1 5E595164
P 1050 2700
F 0 "R2" V 845 2700 50  0000 C CNN
F 1 "0.01" V 936 2700 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 1050 2700 50  0001 C CNN
F 3 "~" H 1050 2700 50  0001 C CNN
	1    1050 2700
	0    1    1    0   
$EndComp
Connection ~ 950  2700
Wire Wire Line
	950  2700 950  2800
Wire Wire Line
	1150 2700 1250 2700
Text GLabel 1300 1950 2    50   Output ~ 0
V_battery
Wire Wire Line
	1250 2700 1250 1950
Wire Wire Line
	1250 1950 1300 1950
Connection ~ 1250 2700
Wire Wire Line
	1250 2700 1500 2700
$Comp
L Device:R_US R5
U 1 1 5E599328
P 1500 3250
F 0 "R5" H 1568 3296 50  0000 L CNN
F 1 "10K" H 1568 3205 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 1540 3240 50  0001 C CNN
F 3 "~" H 1500 3250 50  0001 C CNN
	1    1500 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	1500 3100 1500 3000
$Comp
L power:GND #PWR07
U 1 1 5E59B0F7
P 1500 3400
F 0 "#PWR07" H 1500 3150 50  0001 C CNN
F 1 "GND" H 1505 3227 50  0000 C CNN
F 2 "" H 1500 3400 50  0001 C CNN
F 3 "" H 1500 3400 50  0001 C CNN
	1    1500 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3250 6500 3200 6500
Wire Wire Line
	3200 6500 3200 6400
Wire Wire Line
	3250 6150 3200 6150
Wire Wire Line
	3200 6150 3200 6300
Wire Wire Line
	3200 6150 3200 5850
Connection ~ 3200 6150
Wire Wire Line
	3200 5850 4650 5850
Wire Wire Line
	4650 6150 4650 5850
Wire Wire Line
	8550 4750 8550 4550
$Comp
L power:GND #PWR0102
U 1 1 5E5B12E0
P 8550 4750
F 0 "#PWR0102" H 8550 4500 50  0001 C CNN
F 1 "GND" H 8555 4577 50  0000 C CNN
F 2 "" H 8550 4750 50  0001 C CNN
F 3 "" H 8550 4750 50  0001 C CNN
	1    8550 4750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5E5B0CF1
P 8450 4550
F 0 "#PWR0101" H 8450 4300 50  0001 C CNN
F 1 "GND" H 8455 4377 50  0000 C CNN
F 2 "" H 8450 4550 50  0001 C CNN
F 3 "" H 8450 4550 50  0001 C CNN
	1    8450 4550
	1    0    0    -1  
$EndComp
Text GLabel 8450 2450 0    50   Output ~ 0
Vout
Text GLabel 9800 4250 2    50   Output ~ 0
SCL
Text GLabel 9800 4150 2    50   Output ~ 0
SDA
$Comp
L power:GND #PWR014
U 1 1 5E584B58
P 8650 4550
F 0 "#PWR014" H 8650 4300 50  0001 C CNN
F 1 "GND" H 8655 4377 50  0000 C CNN
F 2 "" H 8650 4550 50  0001 C CNN
F 3 "" H 8650 4550 50  0001 C CNN
	1    8650 4550
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 5E5831A8
P 1500 6300
F 0 "J3" H 1580 6292 50  0000 L CNN
F 1 "Conn_01x02" H 1580 6201 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 1500 6300 50  0001 C CNN
F 3 "~" H 1500 6300 50  0001 C CNN
	1    1500 6300
	-1   0    0    1   
$EndComp
$Comp
L Device:R_US R10
U 1 1 5E587E43
P 1750 6600
F 0 "R10" H 1818 6646 50  0000 L CNN
F 1 "10K" H 1818 6555 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 1790 6590 50  0001 C CNN
F 3 "~" H 1750 6600 50  0001 C CNN
	1    1750 6600
	1    0    0    -1  
$EndComp
$Comp
L Connector:Micro_SD_Card J4
U 1 1 5E58A0E2
P 5300 3850
F 0 "J4" H 5250 3033 50  0000 C CNN
F 1 "Micro_SD_Card" H 5250 3124 50  0000 C CNN
F 2 "Connector_Card:microSD_HC_Molex_104031-0811" H 6450 4150 50  0001 C CNN
F 3 "http://katalog.we-online.de/em/datasheet/693072010801.pdf" H 5300 3850 50  0001 C CNN
	1    5300 3850
	-1   0    0    1   
$EndComp
NoConn ~ 6200 3450
NoConn ~ 6200 4150
$Comp
L power:GND #PWR0103
U 1 1 5E5917B2
P 6300 4150
F 0 "#PWR0103" H 6300 3900 50  0001 C CNN
F 1 "GND" H 6305 3977 50  0000 C CNN
F 2 "" H 6300 4150 50  0001 C CNN
F 3 "" H 6300 4150 50  0001 C CNN
	1    6300 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 3650 6300 3650
Wire Wire Line
	6300 3650 6300 4150
Wire Wire Line
	6350 3850 6200 3850
Wire Wire Line
	8050 3950 6200 3950
Wire Wire Line
	8050 4150 6850 4150
Wire Wire Line
	6850 4150 6850 3750
Wire Wire Line
	6850 3750 6200 3750
Wire Wire Line
	7450 3850 8050 3850
Wire Wire Line
	6200 4050 7450 4050
Wire Wire Line
	7750 3550 7750 4050
Wire Wire Line
	7750 4050 8050 4050
Wire Wire Line
	3100 2300 2700 2300
Wire Wire Line
	3100 2650 3100 2300
Wire Wire Line
	3300 4850 3600 4850
Wire Wire Line
	2050 4850 2250 4850
Wire Wire Line
	7450 4050 7450 3850
$Comp
L MCU_Module:Arduino_UNO_R3 A1
U 1 1 5E56F4DD
P 8550 3450
F 0 "A1" H 8550 4631 50  0000 C CNN
F 1 "Arduino_UNO_R3" H 8550 4540 50  0000 C CNN
F 2 "Module:Arduino_UNO_R3" H 8550 3450 50  0001 C CIN
F 3 "https://www.arduino.cc/en/Main/arduinoBoardUno" H 8550 3450 50  0001 C CNN
	1    8550 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 3200 3100 3200
Wire Wire Line
	2700 2600 7750 2600
$Comp
L power:GND #PWR0104
U 1 1 5E5A8ADA
P 4300 3350
F 0 "#PWR0104" H 4300 3100 50  0001 C CNN
F 1 "GND" H 4305 3177 50  0000 C CNN
F 2 "" H 4300 3350 50  0001 C CNN
F 3 "" H 4300 3350 50  0001 C CNN
	1    4300 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	4500 3250 4300 3250
Wire Wire Line
	4300 3250 4300 3350
$Comp
L Device:C C2
U 1 1 5E5AB985
P 2250 5000
F 0 "C2" H 2365 5046 50  0000 L CNN
F 1 "1UF" H 2365 4955 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 2288 4850 50  0001 C CNN
F 3 "~" H 2250 5000 50  0001 C CNN
	1    2250 5000
	1    0    0    -1  
$EndComp
Connection ~ 2250 4850
Wire Wire Line
	2250 4850 2350 4850
$Comp
L power:GND #PWR0105
U 1 1 5E5AC407
P 2250 5200
F 0 "#PWR0105" H 2250 4950 50  0001 C CNN
F 1 "GND" H 2255 5027 50  0000 C CNN
F 2 "" H 2250 5200 50  0001 C CNN
F 3 "" H 2250 5200 50  0001 C CNN
	1    2250 5200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2250 5150 2250 5200
$Comp
L Connector:Screw_Terminal_01x02 J5
U 1 1 5E5AEA7F
P 2250 4300
F 0 "J5" V 2214 4112 50  0000 R CNN
F 1 "Screw_Terminal_01x02" V 2123 4112 50  0000 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 2250 4300 50  0001 C CNN
F 3 "~" H 2250 4300 50  0001 C CNN
	1    2250 4300
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2250 4500 2250 4850
$Comp
L power:GND #PWR0106
U 1 1 5E5B1844
P 2350 4500
F 0 "#PWR0106" H 2350 4250 50  0001 C CNN
F 1 "GND" H 2355 4327 50  0000 C CNN
F 2 "" H 2350 4500 50  0001 C CNN
F 3 "" H 2350 4500 50  0001 C CNN
	1    2350 4500
	1    0    0    -1  
$EndComp
Wire Wire Line
	8650 2450 8650 2000
Wire Wire Line
	8650 2000 6350 2000
Wire Wire Line
	6350 2000 6350 3850
$Comp
L Device:R_US R11
U 1 1 5E5BB3EC
P 9650 4000
F 0 "R11" H 9718 4046 50  0000 L CNN
F 1 "4.7K" H 9718 3955 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 9690 3990 50  0001 C CNN
F 3 "~" H 9650 4000 50  0001 C CNN
	1    9650 4000
	1    0    0    -1  
$EndComp
$Comp
L Device:R_US R12
U 1 1 5E5BC270
P 9300 4100
F 0 "R12" H 9232 4054 50  0000 R CNN
F 1 "4.7K" H 9232 4145 50  0000 R CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 9340 4090 50  0001 C CNN
F 3 "~" H 9300 4100 50  0001 C CNN
	1    9300 4100
	-1   0    0    1   
$EndComp
Wire Wire Line
	9800 4250 9300 4250
Connection ~ 9300 4250
Wire Wire Line
	9300 4250 9050 4250
Wire Wire Line
	9800 4150 9650 4150
Connection ~ 9650 4150
Wire Wire Line
	9650 4150 9050 4150
Wire Wire Line
	9650 3850 9650 2000
Wire Wire Line
	9650 2000 9300 2000
Connection ~ 8650 2000
Wire Wire Line
	9300 3950 9300 2000
Connection ~ 9300 2000
Wire Wire Line
	9300 2000 8650 2000
Wire Wire Line
	1700 6200 1700 6000
Wire Wire Line
	2000 6700 2000 6800
Wire Wire Line
	1750 6750 1750 6800
Wire Wire Line
	1750 6800 1900 6800
Connection ~ 2000 6800
Connection ~ 1900 6800
Wire Wire Line
	1900 6800 2000 6800
Text GLabel 1700 6000 2    50   Input ~ 0
V_battery
Wire Wire Line
	1700 6300 1750 6300
Wire Wire Line
	1750 6450 1750 6300
Connection ~ 1750 6300
Wire Wire Line
	1750 6300 2000 6300
$Comp
L Transistor_BJT:2N3904 Q1
U 1 1 5E6223E2
P 9200 5150
F 0 "Q1" H 9390 5196 50  0000 L CNN
F 1 "2N3904" H 9390 5105 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 9400 5075 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N3904.pdf" H 9200 5150 50  0001 L CNN
	1    9200 5150
	1    0    0    -1  
$EndComp
Wire Wire Line
	8050 3750 7900 3750
$Comp
L Device:R_US R6
U 1 1 5E634871
P 8300 5150
F 0 "R6" V 8095 5150 50  0000 C CNN
F 1 "180" V 8186 5150 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 8340 5140 50  0001 C CNN
F 3 "~" H 8300 5150 50  0001 C CNN
	1    8300 5150
	0    1    1    0   
$EndComp
Wire Wire Line
	7900 5150 8150 5150
Wire Wire Line
	7900 3750 7900 5150
Wire Wire Line
	6200 3550 7750 3550
Wire Wire Line
	7750 2600 7750 3050
Wire Wire Line
	7750 3050 8050 3050
Wire Wire Line
	8050 3150 7200 3150
$Comp
L Connector:Conn_01x04_Male J6
U 1 1 5E6625BD
P 7300 5500
F 0 "J6" V 7454 5212 50  0000 R CNN
F 1 "Conn_01x04_Male" V 7363 5212 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 7300 5500 50  0001 C CNN
F 3 "~" H 7300 5500 50  0001 C CNN
	1    7300 5500
	0    -1   -1   0   
$EndComp
Wire Wire Line
	7300 3250 8050 3250
Wire Wire Line
	8050 3350 7400 3350
$Comp
L Device:R_US R13
U 1 1 5E68CAEC
P 7200 4350
F 0 "R13" H 7268 4396 50  0000 L CNN
F 1 "470" H 7268 4305 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0309_L9.0mm_D3.2mm_P12.70mm_Horizontal" V 7240 4340 50  0001 C CNN
F 3 "~" H 7200 4350 50  0001 C CNN
	1    7200 4350
	1    0    0    -1  
$EndComp
$Comp
L Device:R_US R14
U 1 1 5E68E409
P 7300 4650
F 0 "R14" H 7368 4696 50  0000 L CNN
F 1 "470" H 7368 4605 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0309_L9.0mm_D3.2mm_P12.70mm_Horizontal" V 7340 4640 50  0001 C CNN
F 3 "~" H 7300 4650 50  0001 C CNN
	1    7300 4650
	1    0    0    -1  
$EndComp
$Comp
L Device:R_US R15
U 1 1 5E68EA86
P 7400 4950
F 0 "R15" H 7468 4996 50  0000 L CNN
F 1 "470" H 7468 4905 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0309_L9.0mm_D3.2mm_P12.70mm_Horizontal" V 7440 4940 50  0001 C CNN
F 3 "~" H 7400 4950 50  0001 C CNN
	1    7400 4950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0108
U 1 1 5E68FCB4
P 7500 5300
F 0 "#PWR0108" H 7500 5050 50  0001 C CNN
F 1 "GND" H 7505 5127 50  0000 C CNN
F 2 "" H 7500 5300 50  0001 C CNN
F 3 "" H 7500 5300 50  0001 C CNN
	1    7500 5300
	-1   0    0    1   
$EndComp
Wire Wire Line
	7200 3150 7200 4200
Wire Wire Line
	7200 4500 7200 5300
Wire Wire Line
	7300 3250 7300 4500
Wire Wire Line
	7300 4800 7300 5300
Wire Wire Line
	7400 4800 7400 3350
Wire Wire Line
	7400 5100 7400 5300
Text GLabel 9300 4450 2    50   Input ~ 0
Vout
$Comp
L Device:Buzzer BZ1
U 1 1 5E6CF8C7
P 9400 4750
F 0 "BZ1" H 9552 4779 50  0000 L CNN
F 1 "Buzzer" H 9552 4688 50  0000 L CNN
F 2 "Buzzer_Beeper:MagneticBuzzer_ProSignal_ABT-410-RC" V 9375 4850 50  0001 C CNN
F 3 "~" V 9375 4850 50  0001 C CNN
	1    9400 4750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0107
U 1 1 5E6D7ECC
P 9300 5900
F 0 "#PWR0107" H 9300 5650 50  0001 C CNN
F 1 "GND" H 9305 5727 50  0000 C CNN
F 2 "" H 9300 5900 50  0001 C CNN
F 3 "" H 9300 5900 50  0001 C CNN
	1    9300 5900
	1    0    0    -1  
$EndComp
Wire Wire Line
	8450 5150 9000 5150
Wire Wire Line
	9300 4450 9300 4650
Wire Wire Line
	9300 4850 9300 4950
Wire Wire Line
	9300 5350 9300 5900
$Comp
L Mechanical:MountingHole H1
U 1 1 5E5F4526
P 600 600
F 0 "H1" H 700 646 50  0000 L CNN
F 1 "MountingHole" H 700 555 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.2mm_M2" H 600 600 50  0001 C CNN
F 3 "~" H 600 600 50  0001 C CNN
	1    600  600 
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H2
U 1 1 5E5F507D
P 1350 600
F 0 "H2" H 1450 646 50  0000 L CNN
F 1 "MountingHole" H 1450 555 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.2mm_M2" H 1350 600 50  0001 C CNN
F 3 "~" H 1350 600 50  0001 C CNN
	1    1350 600 
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H3
U 1 1 5E5F5910
P 2100 600
F 0 "H3" H 2200 646 50  0000 L CNN
F 1 "MountingHole" H 2200 555 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.2mm_M2" H 2100 600 50  0001 C CNN
F 3 "~" H 2100 600 50  0001 C CNN
	1    2100 600 
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H4
U 1 1 5E5F5CCC
P 2850 600
F 0 "H4" H 2950 646 50  0000 L CNN
F 1 "MountingHole" H 2950 555 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.2mm_M2" H 2850 600 50  0001 C CNN
F 3 "~" H 2850 600 50  0001 C CNN
	1    2850 600 
	1    0    0    -1  
$EndComp
$EndSCHEMATC
