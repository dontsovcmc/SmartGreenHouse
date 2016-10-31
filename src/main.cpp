#include "Arduino.h"
#include "stdlib.h"

#include "pins.h"

#include "kran.h"

#include <TimeAlarms.h>

#define ONLY_UP_KEY
#include <menu.h>
#include <menuLCDs.h>
#include <menuFields.h>
#include <keyStream.h>
#include <chainStream.h>// concatenate multiple input streams (this allows adding a button to the encoder)

#include "settings.h"
#include "sgh_time.h"
#include "sgh_lcd.h"
#include "sgh_menu.h"

#include "multidesktop.h"

char alarms_id[ALARMS]; 

Kran kran(OPEN_PIN, CLOSE_PIN, LED_KRAN, OPEN_TIME_MSEC);

// Menu data
extern menuLCD menu_lcd;
extern chainStream<2> allIn;

#define BUF_LEN 32
char buf__[BUF_LEN];  //for printf 

unsigned long blink_time = 0;
bool blink_on = false;

bool poll_menu = false;

void logln(const char *str) { Serial.println(str); }

extern "C" {
  typedef void (*alarmFunction)(void);
}


void screen_info(const char *buffer, int delay)
{
	logln(buffer);
	print_message(buffer);
	
	Alarm.delay(delay);
	lcd.clear();
}

void start_kran(int seconds)
{
    screen_info("POLIV START", 500);
    kran.set_duration(seconds*1000);
    kran.start_work();
}


void turn_off_relay1()
{
	logln("relay1 OFF");
	digitalWrite(RELAY1_PIN, LOW);	
}

void turn_off_relay2()
{
	logln("relay2 OFF");
	digitalWrite(RELAY2_PIN, LOW);	
}

void turn_on_relay1(int duration)
{
	logln("relay1 ON");
	digitalWrite(RELAY1_PIN, HIGH);
	Alarm.timerOnce(duration, turn_off_relay1); 
}

void turn_on_relay2(int duration)
{
	logln("relay2 ON");
	digitalWrite(RELAY2_PIN, HIGH);
	Alarm.timerOnce(duration, turn_off_relay2); 
}

void alarm_function(char id)
{
	switch(settings.alarms[id].alarm_type)
	{
		case POLIV_TYPE:
			start_kran(settings.alarms[id].alarm_duration);
		break;
		case RELAY1_TYPE:
			turn_on_relay1(settings.alarms[id].alarm_duration);
		break;
		case RELAY2_TYPE:
		    turn_on_relay2(settings.alarms[id].alarm_duration);
		break;	
	}
}

void alarm0() { alarm_function(0); }
void alarm1() { alarm_function(1); }
void alarm2() { alarm_function(2); }
void alarm3() { alarm_function(3); }
void alarm4() { alarm_function(4); }

alarmFunction alarm_func[ALARMS] = { &alarm0, &alarm1, &alarm2, &alarm3, &alarm4};


void blink() //LED_WORK
{
    if (kran.opened())
    {
        digitalWrite(LED_WORK, HIGH);
        return;
    }

    if (millis() - blink_time > BLINK_TIME)
    {
      if (blink_on)
      {
          blink_on = false;
          digitalWrite(LED_WORK, blink_on);
      }
    }

    if (millis() - blink_time > BLINK_DELAY)
    {
        if (!blink_on)
        {
            blink_on = true;
            digitalWrite(LED_WORK, blink_on);
            blink_time = millis();
        }
    }
}

void start_menu()
{
	poll_menu = true;
}

void setup_alarms()
{
	for (int i=0; i < ALARMS; i++)
	{
		if (settings.alarms[i].alarm_enable)
		{
			if (alarms_id[i] == -1)
			{
				alarms_id[i] = Alarm.alarmRepeat(settings.alarms[i].alarm_hour, settings.alarms[i].alarm_min, 0, alarm_func[i]);
			}
			else
			{
				Alarm.write(alarms_id[i], AlarmHMS(settings.alarms[i].alarm_hour, settings.alarms[i].alarm_min, 0));
			}
			
			snprintf (buf__, BUF_LEN, "TIMER %d: %02d:%02d", i+1, settings.alarms[i].alarm_hour, settings.alarms[i].alarm_min);
			screen_info(buf__, 500);
		}
	}
}

void load_settings()
{
	char ver = read_settings();
	if (ver > 0)
	{
		snprintf (buf__, BUF_LEN, "SETT. LOADED %d", ver);
		screen_info(buf__, 500);
	}
	else
	{
		screen_info("SETT. ERROR", 1000);
	}

	set_time();  ///??
	setup_alarms();
}


void start_screen()
{
 	logln("start_screen");
	
	if (kran.opened())
	{
		int sec = 	kran.poliv_left_sec();
		sprintf (&buf__[0], "POLIV: %d         ", sec);
		print_screen(buf__, "STOP", nullptr);
	}
	else
	{
		get_rtc_time_str(buf__, BUF_LEN);
		print_screen(buf__, "POLIV", ">TIMER");
	}
}

void start_screen_action()
{
	if (kran.opened())
	{
		screen_info("POLIV STOPPED", 500);
        kran.stop_work();
	}
	else
	{
		start_kran(POLIV_MANUAL_LONG);
	}
}


void screen2()
{
	//float t = -1.1;
	//char t_s[6];
	//dtostrf(t, 4, 1, t_s);
	//snprintf (&buf__[0], BUF_LEN, "T=%sC           ", t_s);
	
	digitalRead(RELAY1_PIN) ? print_screen("RELAY", "OFF", ">MENU") : print_screen("RELAY", "ON", ">MENU");
}


void toggle_relay1() { digitalWrite(RELAY1_PIN, !digitalRead(RELAY1_PIN)); }


bool reset_settings()
{
	init_settings();
	write_settings();
	screen_info("Settings\nreset OK", 500);
	return false;
}

void menu_screen()
{
	time_t next = Alarm.getNextTrigger();
	time_t elapsed = next - now();
	snprintf (buf__, BUF_LEN, "poliv: %02d:%02d:%02d", hour(elapsed), minute(elapsed), second(elapsed));
	print_screen(buf__, "MENU", "> POLIV");
}


bool close_settings_menu()
{
	poll_menu = false;
	
	write_settings();
	screen_info("Settings saved", 500);
	load_settings();
	
	start_screen();
	return true;
}

promptFeedback quit() { return true; }
promptFeedback cancel() { return true; }
promptFeedback setCurAlarm();
promptFeedback setAlarmSett();

alarm_settings alarm_sett;

TOGGLE(alarm_sett.alarm_enable,alarm_on_menu,"Work: ",
  VALUE("YES",1),
  VALUE("NO",0)
);

TOGGLE(alarm_sett.alarm_type,alarm_type_menu,"Type: ",
  VALUE("Poliv",POLIV_TYPE),
  VALUE("Relay 1",RELAY1_TYPE),
  VALUE("Relay 2",RELAY2_TYPE)
);

MENU(alarm_menu,"Alarm def"
  , SUBMENU(alarm_on_menu)
  , SUBMENU(alarm_type_menu)
  , FIELD(alarm_sett.alarm_duration,"Dlit","sec", 1, 60, 5, 1)
  , FIELD(alarm_sett.alarm_hour,"Start","hour", 0, 23, 1, 1)
  , FIELD(alarm_sett.alarm_min,"Start","min", 0, 59, 10, 1)
  , OP("OK", setAlarmSett)
);

MENU(mainMenu,"Main"
  , OP("Alarm 1",setCurAlarm)
  , OP("Alarm 2",setCurAlarm)
  , OP("Alarm 3",setCurAlarm)
  , OP("Alarm 4",setCurAlarm)
  , OP("Alarm 5",setCurAlarm)
  , OP("Reset", reset_settings)
  , OP("Exit", close_settings_menu)
);

promptFeedback setCurAlarm() {
  alarm_sett = settings.alarms[mainMenu.sel];
  menuNode::activeNode = &alarm_menu;
  return false;
}

promptFeedback setAlarmSett() {
  settings.alarms[mainMenu.sel] = alarm_sett;
  return cancel();
}

void open_menu()
{
	poll_menu = true;
	mainMenu.sel = 0; //0 reset the menu index for next call
	logln("open menu");
}

//Multi Desktop configuration
#define BUTTONS 1
int8_t codes[] = { menu::upCode };

func desktop1cb[] = { &start_screen_action };
func desktop2cb[] = { &toggle_relay1 };
func desktop3cb[] = { &open_menu };

MultiDesktop<BUTTONS> multi_desktop( menu::enterCode , codes);
Desktop<BUTTONS> mainDesktop(&start_screen, desktop1cb); 
Desktop<BUTTONS> secondDesktop(&screen2, desktop2cb);
Desktop<BUTTONS> menuDesktop(&menu_screen, desktop3cb);


void pin_setup()
{
	pinMode(LED_WORK, OUTPUT);
	pinMode(BUTTON_1,INPUT);
	pinMode(BUTTON_2,INPUT); 
	pinMode(RELAY1_PIN, OUTPUT); 
	pinMode(RELAY2_PIN, OUTPUT); 
	
	digitalWrite(LED_WORK, LOW);
	
	digitalWrite(BUTTON_1,LOW);
	digitalWrite(BUTTON_2,LOW);
	
	digitalWrite(RELAY1_PIN,LOW);	
	digitalWrite(RELAY2_PIN,LOW);
}

time_t sync_time()
{
	screen_info("TIME SYNC", 1000);
	return RTC.get();
}

void setup()
{
	for (int i=0; i < ALARMS; i++, alarms_id[i] = -1) ;
	
	kran.setup();
	pin_setup();
	
	lcd.begin(16,2);  
	Serial.begin(9600);

	load_time(buf__, BUF_LEN);
	screen_info(buf__, 1000);
	
	//debug ---> !!!
	/*
	settings.alarm_enable[0] = true;
	settings.alarm_type[0] = POLIV_TYPE;
	settings.alarm_hour[0] = 20;
	settings.alarm_min[0] = 19;
	settings.alarm_duration[0] = 10;
	
	settings.alarm_enable[1] = true;
	settings.alarm_type[1] = RELAY1_TYPE;
	settings.alarm_hour[1] = 20;
	settings.alarm_min[1] = 20;
	settings.alarm_duration[1] = 10;
	
	write_settings();*/
	//< --- debug !!!
	
	load_settings();
	
	setSyncProvider(&sync_time);
    setSyncInterval(60*60*24); 
	
	multi_desktop.add_desktop(&mainDesktop);
	multi_desktop.add_desktop(&secondDesktop);
	multi_desktop.add_desktop(&menuDesktop);
	
	multi_desktop.show();
}


void loop() 
{
	if (poll_menu)
	{
		mainMenu.poll(menu_lcd, allIn);
	}
	else 
	{
		if (allIn.available()) 
		{
			char ch = allIn.read();
			multi_desktop.button_pressed(ch);
		}
		
		multi_desktop.show(); 
	}
	
	kran.poll();
	blink();
	Alarm.delay(1); // delay in between reads for stability
	
}
