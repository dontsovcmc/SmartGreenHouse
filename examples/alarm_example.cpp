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

#define LED_WORK  2 
#define LED_KRAN  3
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

AlarmId id;
char buf__[32];

// functions to be called when an alarm triggers:
void AlarmFunc() {
  Serial.println("Alarm: - turn lights off");
  digitalWrite(LED_KRAN, !digitalRead(LED_KRAN));
  
	lcd.setCursor(0,0); 
	lcd.print("AlarmFunc        ");
	
	tmElements_t tm;
	if (RTC.read(tm)) 
	{
		sprintf (buf__, "%02d/%02d %02d:%02d:%02d", tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
		lcd.setCursor(0,1);
		lcd.print(buf__);
	}
	
}

void Repeats2() {
  digitalWrite(LED_WORK, !digitalRead(LED_WORK));
  Serial.println("2 second timer");
  
	lcd.setCursor(0,0); 
	lcd.print("2 sec         ");
}

void OnceOnly() {
	
	lcd.setCursor(0,0); 
	lcd.print("OnceOnly         ");
  digitalWrite(LED_KRAN, !digitalRead(LED_KRAN));
  
  Serial.println("This timer only triggers once, stop the 2 second timer");
  // use Alarm.free() to disable a timer and recycle its memory.
  Alarm.free(id);
  // optional, but safest to "forget" the ID after memory recycled
  id = dtINVALID_ALARM_ID;
  // you can also use Alarm.disable() to turn the timer off, but keep
  // it in memory, to turn back on later with Alarm.enable().
}

void setup() {
	
	pinMode(LED_WORK, OUTPUT);
	pinMode(LED_KRAN, OUTPUT);
  Serial.begin(9600);
  while (!Serial) ; // wait for Arduino Serial Monitor

	sprintf (buf__, "TIME");
	
	lcd.begin(16,2);  
	lcd.clear();	
	lcd.setCursor(0,0); 
	lcd.print(buf__);
	tmElements_t tm;
	if (RTC.read(tm)) 
	{
		setTime(tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tmYearToY2k(tm.Year)); 
		sprintf (buf__, "%02d/%02d %02d:%02d:%02d", tm.Month, tm.Day, tm.Hour, tm.Minute, tm.Second);
		lcd.setCursor(0,1);
		lcd.print(buf__);
	}
	
	
  // create the alarms, to trigger at specific times
  Alarm.alarmRepeat(14,00,0, AlarmFunc);     // 8:30am every day
  id = Alarm.timerRepeat(2, Repeats2);      // timer for every 2 seconds
  Alarm.timerOnce(10, OnceOnly);            // called once after 10 seconds
}

void loop() {
  time_t t = now();
  sprintf (buf__, "%02d/%02d %02d:%02d:%02d", month(t), day(t), hour(t), minute(t), second(t));
	lcd.setCursor(0,1);
	lcd.print(buf__);
  
  Alarm.delay(5); // wait one second between clock display
}


