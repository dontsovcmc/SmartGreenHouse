/*
 * TimeAlarmExample.pde
 *
 * This example calls alarm functions at 8:30 am and at 5:45 pm (17:45)
 * and simulates turning lights on at night and off in the morning
 * A weekly timer is set for Saturdays at 8:30:30
 *
 * A timer is called every 15 seconds
 * Another timer is called once only after 10 seconds
 *
 * At startup the time is set to Jan 1 2011  8:29 am
 */

// Questions?  Ask them here:
// http://forum.arduino.cc/index.php?topic=66054.0

#include <TimeLib.h>
#include <TimeAlarms.h>

#include "wire.h"
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

AlarmId id;
#define BUF_LEN 32
char buf__[BUF_LEN];

void message(char *buf)
{
	Serial.println(buf);
	
	char *line2 = strchr(buf, '\n');
	if (line2) *line2 = '\0';
	
	lcd.setCursor(0, 0); 
	lcd.print(buf);
	for (unsigned int i = 0; i < 16 - strlen(buf); i++) lcd.print(" "); 
	lcd.setCursor(0, 1);
	
	if (line2) 
	{ 
		lcd.print(++line2);
		for (unsigned int i = 0; i < 16 - strlen(line2); i++) lcd.print(" "); 
	}
}

void AlarmFunc() 
{
	message("AlarmFunc");
	Alarm.delay(1000);
}


void message_now()
{
	time_t t = now();
	snprintf (buf__, BUF_LEN, "NOW:%02d/%02d/%02d\n%02d:%02d:%02d", 
		year(t), month(t), day(t), hour(t), minute(t), second(t));
	message(buf__);
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

	snprintf (buf__, BUF_LEN, "YEARF: %d   \n tm:%d    ", Year, tm->Year);
	message(buf__);
	Alarm.delay(1500);
	
  return true;
}

void setup() 
{
	Serial.begin(9600);
	while (!Serial) ; 
	
	lcd.begin(16,2); 
	lcd.clear();	
	
	snprintf (buf__, BUF_LEN, "FILE:%s\n%s", 
		__TIME__, __DATE__);
	message(buf__);
	Alarm.delay(1500);
	
	tmElements_t tm;
	if (getDate(&tm, __DATE__) && getTime(&tm, __TIME__)) 
	{
		if (RTC.write(tm)) 
		{
			message("WRITE TIME OK");
		}
	}
	
	if (RTC.read(tm)) 
	{
		setTime(tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tmYearToCalendar(tm.Year)); 
		snprintf (buf__, BUF_LEN, "RTC:%02d/%02d/%02d\n%02d:%02d:%02d", 
			tmYearToCalendar(tm.Year), tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
		message(buf__);
		Alarm.delay(1500);
	}

	// create the alarms, to trigger at specific times
	Alarm.alarmRepeat(20,00, 0, AlarmFunc);     // 00:00 every day
	//id = Alarm.timerRepeat(2, Repeats2);      // timer for every 2 seconds
	//Alarm.timerOnce(10, OnceOnly);            // called once after 10 seconds
}

void loop() 
{
	if (second() / 2 % 2)
	{
		time_t e = Alarm.getNextTrigger() - now();
		snprintf (buf__, BUF_LEN, "next:%02d/%02d/%02d\n%02d:%02d:%02d", 
			year(e), month(e), day(e), hour(e), minute(e), second(e));
		message(buf__);
	}
	else
	{
		message_now();
	}
	Alarm.delay(1000); // wait one second between clock display
}


