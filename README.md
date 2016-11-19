# SmartGreenHouse

Board: Arduino UNO 

Aim: do something by alarm scheduler

Current version features:
a. show temperature
b. electric ball Valve control (manual and alarm)
c. turn on Relay (manual and alarm)
Remember settings in EEPROM

# Quick start

1. install platformio
2. change COM port in platformio.ini file here
3. run init.sh to download external libraries
4. download NewLiquidCrystal library manual to ./library folder
5. run run.sh to compile and upload sketch to arduino
6. if #define LOGGING, you can see debug info reading COM port. run serial.sh

## Electric ball Valve control

Valve has 3 pins(GND, open, close) by default 12V.
But the power of Arduino is 5V. I invert power by solder and got PWR, open, close. Now I can use Transistor to move it.
Arduino pins open Transistors and got GND on open or close pin.

Algorith:
1. open pin turn on for 5 seconds (motor works).
2. open pin turn off. Valve is open.
3. enjoy
4. closing pin turn on for 5 seconds (motor works).
5. closing pin turn off. Valse is close.


# Additional libraries

## Hardware

1. NewliquidCrystal - LCD 16x2 i2c display driver
2. OneWire - 1-wire device support
3. Arduino-Temperature-Control-Library - Dallas DS18B20 temperature sensor driver
4. DS1307RTC - real time clock driver

+ read/write EEPROM

## Software

1. ArduinoMultiDesktop - easy 2 buttons change screen library
2. ArduinoMenu - great library for print menu settings
3. TimeAlarms - alarm library
4. Time - needs to alarm library for time functions
