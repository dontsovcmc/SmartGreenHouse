
#ifndef POLIV_SETTINGS_H
#define POLIV_SETTINGS_H

#include <avr/eeprom.h>

#define MEMORY_ADDR 120

#define SETTINGS_VER1 1
#define SETTINGS_VER2 2
#define SETTINGS_CURRENT SETTINGS_VER2

#define POLIV_TYPE 0
#define RELAY1_TYPE 1
#define RELAY2_TYPE 2

struct PolivSettings_1
{
	bool poliv_enable;
	int poliv_run_hour;
	int poliv_run_min;
	int poliv_period;
	int poliv_duration;
	//add new settings here
} settings_1;

#define ALARMS 5
struct PolivSettings_2
{
	bool alarm_enable[ALARMS];
	int  alarm_type[ALARMS];
	int  alarm_hour[ALARMS];
	int  alarm_min[ALARMS];
	int  alarm_duration[ALARMS];
	//add new settings here
} settings;

//for another version add another structures and increase version in init

void init_settings()
{
	settings.alarm_enable[0] = true;
	settings.alarm_type[0] = POLIV_TYPE;
	settings.alarm_hour[0] = 0;
	settings.alarm_min[0] = 0;
	settings.alarm_duration[0] = 10;
	
	settings.alarm_enable[1] = true;
	settings.alarm_type[1] = RELAY1_TYPE;
	settings.alarm_hour[1] = 0;
	settings.alarm_min[1] = 0;
	settings.alarm_duration[1] = 10;
	
	settings.alarm_enable[2] = false;
	settings.alarm_enable[3] = false;
	settings.alarm_enable[4] = false;
}

bool write_settings()
{
	eeprom_write_byte(0, SETTINGS_CURRENT);
	eeprom_write_block((const void*)&settings, (void*)1, sizeof(settings));
	return true;
}

int read_settings()
{
	char version;
	version = eeprom_read_byte(0);
	
	init_settings();
	if (version == SETTINGS_VER1)
	{
		eeprom_read_block((void*)&settings_1, (void*)1, sizeof(PolivSettings_1));	
		settings.alarm_enable[0] = settings_1.poliv_enable;
		settings.alarm_type[0] = POLIV_TYPE;
		settings.alarm_hour[0] = settings_1.poliv_run_hour;
		settings.alarm_min[0] = settings_1.poliv_run_min;
		settings.alarm_duration[0] = settings_1.poliv_duration;
		return 1;
	}
	else if (version == SETTINGS_VER2)
	{
		eeprom_read_block((void*)&settings, (void*)1, sizeof(PolivSettings_2));	
		return 2;
	}
	return false;
}

#endif