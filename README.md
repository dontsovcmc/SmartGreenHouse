# SmartGreenHouse

This device can do something by alarm sheduler.

Current version features:
a. control electric ball Valve (manual and alarm)
b. turn on Relay (manual and alarm)

Remember settings in EEPROM



# Quick start

1. install platformio
2. change COM port in platformio.ini file here
3. run init.sh to download external libraries
4. download NewLiquidCrystal library manual to ./library folder
5. run run.sh to compile and upload sketch to arduino


# Additional libraries

## Hardware

1. NewliquidCrystal - LCD 16x2 i2c display driver
2. OneWire - 1-wire device support
3. Arduino-Temperature-Control-Library - Dallas DS18B20 temperature sensor driver
4. DS1307RTC - real time clock driver

## Software

1. ArduinoMultiDesktop - easy 2 buttons change screen library
2. ArduinoMenu - great library for print menu settings
3. TimeAlarms - alarm library
4. Time - needs to alarm library for time functions
