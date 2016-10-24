
#ifndef SGH_TIME_H
#define SGH_TIME_H

#include "OneButton.h"
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

void get_time_str(char *buf, const int len)
{
	tmElements_t tm;
	if (RTC.read(tm)) {
		sprintf (buf, "%02d/%02d/%02d %02d:%02d", tmYearToCalendar(tm.Year), tm.Month, tm.Day, tm.Hour, tm.Minute);
	}
}

void setup_internal_time()
{
	tmElements_t tm;
	if (RTC.read(tm)) 
	{
		setTime(tm.Hour, tm.Minute, 0, tm.Day, tm.Month, tmYearToY2k(tm.Year)); 
	}
}

bool getTime(tmElements_t *tm, const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  tm->Hour = Hour;
  tm->Minute = Min;
  tm->Second = Sec;
  return true;
}

bool getDate(tmElements_t *tm, const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  tm->Day = Day;
  tm->Month = monthIndex + 1;
  tm->Year = CalendarYrToTm(Year);
  return true;
}


#endif
