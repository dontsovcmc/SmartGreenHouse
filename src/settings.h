
#ifndef POLIV_SETTINGS_H
#define POLIV_SETTINGS_H

#include <avr/eeprom.h>

#define MEMORY_ADDR 120

#define SETTINGS_VER1 1
#define SETTINGS_VER2 2
#define SETTINGS_CURRENT SETTINGS_VER2


struct PolivSettings_1
{
	bool poliv_enable;
	int poliv_run_hour;
	int poliv_run_min;
	int poliv_period;
	int poliv_duration;
	//add new settings here
} settings_1;

struct PolivSettings_2
{
	bool poliv_enable;
	int poliv_run_hour;
	int poliv_run_min;
	int poliv_duration;
	
	bool fan_enable;
	int fan_run_hour;
	int fan_run_min;
	int fan_duration;	
	//add new settings here
} settings;

//for another version add another structures and increase version in init

void init_settings()
{
	settings.poliv_enable = true;
	settings.poliv_run_hour = 21;
	settings.poliv_run_min = 53;
	settings.poliv_duration = 10;
	
	settings.fan_enable = true;
	settings.fan_run_hour = 21;
	settings.fan_run_min = 54;
	settings.fan_duration = 5;
}

bool save_settings()
{
	eeprom_write_byte(0, SETTINGS_CURRENT);
	eeprom_write_block((const void*)&settings, (void*)1, sizeof(settings));
	return true;
}

int load_settings()
{
	char version;
	version = eeprom_read_byte(0);
	
	init_settings();
	if (version == SETTINGS_VER1)
	{
		eeprom_read_block((void*)&settings_1, (void*)1, sizeof(PolivSettings_1));	
		settings.poliv_enable = settings_1.poliv_enable;
		settings.poliv_run_hour = settings_1.poliv_run_hour;
		settings.poliv_run_min = settings_1.poliv_run_min;
		settings.poliv_duration = settings_1.poliv_duration;
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