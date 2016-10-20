
#ifndef POLIV_SETTINGS_H
#define POLIV_SETTINGS_H

#include <avr/eeprom.h>

#define MEMORY_ADDR 120

#define SETTINGS_VER1 1
#define SETTINGS_CURRENT SETTINGS_VER1

//eeprom 1
struct PolivSettings_1
{
	bool poliv_enable;
	int poliv_run_hour;
	int poliv_run_min;
	int poliv_period;
	int poliv_duration;
	//add new settings here
} settings;

//for another version add another structures and increase version in init

void init_settings()
{
	settings.poliv_enable = true;
	settings.poliv_run_hour = 0;
	settings.poliv_run_min = 0;
	settings.poliv_period = 24;
	settings.poliv_duration = 3;
}

bool save_settings()
{
	eeprom_write_byte(0, SETTINGS_CURRENT);
	eeprom_write_block((const void*)&settings, (void*)1, sizeof(settings));
	return true;
}

bool load_settings()
{
	char version;
	version = eeprom_read_byte(0);
	
	init_settings();
	if (version == SETTINGS_VER1)
	{
		eeprom_read_block((void*)&settings, (void*)1, sizeof(PolivSettings_1));	
		return true;
	}
	return false;
}

#endif