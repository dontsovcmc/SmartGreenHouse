
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

struct alarm_settings
{
	bool alarm_enable;
	int  alarm_type;
	int  alarm_hour;
	int  alarm_min;
	int  alarm_duration;
};

#define ALARMS 5
struct PolivSettings_2
{
	struct alarm_settings alarms[ALARMS];
	//add new settings here
} settings;

//for another version add another structures and increase version in init

void init_settings()
{
	settings.alarms[0].alarm_enable = true;
	settings.alarms[0].alarm_type = POLIV_TYPE;
	settings.alarms[0].alarm_hour = 0;
	settings.alarms[0].alarm_min = 0;
	settings.alarms[0].alarm_duration = 10;
	
	settings.alarms[1].alarm_enable = true;
	settings.alarms[1].alarm_type = RELAY1_TYPE;
	settings.alarms[1].alarm_hour = 0;
	settings.alarms[1].alarm_min = 0;
	settings.alarms[1].alarm_duration = 10;
	
	settings.alarms[2].alarm_enable = false;
	settings.alarms[3].alarm_enable = false;
	settings.alarms[4].alarm_enable = false;
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
		settings.alarms[0].alarm_enable = settings_1.poliv_enable;
		settings.alarms[0].alarm_type = POLIV_TYPE;
		settings.alarms[0].alarm_hour = settings_1.poliv_run_hour;
		settings.alarms[0].alarm_min = settings_1.poliv_run_min;
		settings.alarms[0].alarm_duration = settings_1.poliv_duration;
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