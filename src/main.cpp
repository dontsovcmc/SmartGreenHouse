#include "Arduino.h"
#include "stdlib.h"

#include "pins.h"

#include "kran.h"

#include <TimeAlarms.h>

#include <menu.h>
#include <menuLCDs.h>
#include <menuFields.h>
#include <keySafeStream.h>
#include <chainStream.h>// concatenate multiple input streams (this allows adding a button to the encoder)

#include "settings.h"
#include "sgh_time.h"
#include "sgh_lcd.h"
#include "sgh_menu.h"

char alarms_id[ALARMS]; 

Kran kran(OPEN_PIN, CLOSE_PIN, LED_KRAN, OPEN_TIME_MSEC);

// Menu data
extern menuLCD menu_lcd;
extern chainStream<2> allIn;

#define BUF_LEN 32
char buf__[BUF_LEN];  //for printf 

unsigned long blink_time = 0;
bool blink_on = false;

bool relay1 = false;
bool poll_menu = false;

void logln(const char *str) { Serial.println(str); }

extern "C" {
  typedef void (*alarmFunction)(void);
}


void print_screen(const char *message, const char *button1, const char *button2)
{
    lcd.setCursor(0,0); 
    (message) ? lcd.print(message) : lcd.print("                ");
	lcd.setCursor(0,1); 
    if (button1)
    {
        lcd.print("[");
        lcd.print(button1);
        lcd.print("]");
    }
    
    for (unsigned int i = 0; i < 16 - strlen(button1) - strlen(button2) - 4; i++)
        lcd.print(" ");        
    
    if (button2)
    {
        lcd.print("[");
        lcd.print(button2);
        lcd.print("]");
    }
	else
	{
		lcd.print("  ");
	}
}

void screen_info(const char *buffer, int delay)
{
	logln(buffer);
	print_screen(buffer, nullptr, nullptr);
	Alarm.delay(delay);
	lcd.clear();		
}

void turn_kran(int seconds)
{
    if (kran.opened())
    {
	   screen_info("POLIV STOPPED", 500);
       kran.stop_work();
    }
    else
    {
	   screen_info("POLIV START", 500);
	   kran.set_duration(seconds*1000);
       kran.start_work();
    }
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
	switch(settings.alarm_type[id])
	{
		case POLIV_TYPE:
			turn_kran(settings.alarm_duration[id]);
		break;
		case RELAY1_TYPE:
			turn_on_relay1(settings.alarm_duration[id]);
		break;
		case RELAY2_TYPE:
		    turn_on_relay2(settings.alarm_duration[id]);
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
    logln("start_menu");
}

void setup_alarms()
{
	for (int i=0; i < ALARMS; i++)
	{
		if (settings.alarm_enable[i])
		{
			if (alarms_id[i] == -1)
			{
				alarms_id[i] = Alarm.alarmRepeat(settings.alarm_hour[i], settings.alarm_min[i], 0, alarm_func[i]);
			}
			else
			{
				Alarm.write(alarms_id[i], AlarmHMS(settings.alarm_hour[i], settings.alarm_min[i], 0));
			}
			
			snprintf (buf__, BUF_LEN, "TIMER %d: %02d:%02d", i+1, settings.alarm_hour[i], settings.alarm_min[i]);
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


bool start_screen()
{
	poll_menu = false;
	
	if (second() / 2 % 2)
	{
		get_rtc_time_str(buf__, BUF_LEN);	
	}
	else
	{
		//float t = -1.1;
		//char t_s[6];
		//dtostrf(t, 4, 1, t_s);
		//snprintf (&buf__[0], BUF_LEN, "T=%sC           ", t_s);
		time_t next = Alarm.getNextTrigger();
		time_t elapsed = next - now();
		snprintf (buf__, BUF_LEN, "next: %02d:%02d:%02d", hour(elapsed), minute(elapsed), second(elapsed));
	}
	
	print_screen(buf__, "POLIV", "MENU");
    
	logln("start_screen");
	
	return true;
}

bool reset_settings()
{
	init_settings();
	write_settings();
	return false;
}

bool close_settings_menu()
{
	write_settings();
	screen_info("Settings saved", 500);
	load_settings();
	
	start_screen();
}

promptFeedback quit() {
  return true;
}

TOGGLE(settings.alarm_enable[0],timer1_on_menu,"Work: ",
  VALUE("YES",1),
  VALUE("NO",0)
);

TOGGLE(settings.alarm_type[0],timer1_type_menu,"Type: ",
  VALUE("Poliv",POLIV_TYPE),
  VALUE("Relay 1",RELAY1_TYPE),
  VALUE("Relay 2",RELAY2_TYPE)
);

MENU(timer1_menu,"Timer 1"
  , SUBMENU(timer1_on_menu)
  , SUBMENU(timer1_type_menu)
  , FIELD(settings.alarm_duration[0],"Dlit","sec", 1, 60, 5, 1)
  , FIELD(settings.alarm_hour[0],"Start","hour", 0, 23, 1, 1)
  , FIELD(settings.alarm_min[0],"Start","min", 0, 59, 10, 1)
  , OP("Exit", quit)
);

TOGGLE(settings.alarm_enable[1],timer2_on_menu,"Work: ",
  VALUE("YES",1),
  VALUE("NO",0)
);

TOGGLE(settings.alarm_type[1],timer2_type_menu,"Type: ",
  VALUE("Poliv",POLIV_TYPE),
  VALUE("Relay 1",RELAY1_TYPE),
  VALUE("Relay 2",RELAY2_TYPE)
);

MENU(timer2_menu,"Timer 2"
  , SUBMENU(timer2_on_menu)
  , SUBMENU(timer2_type_menu)
  , FIELD(settings.alarm_duration[1],"Dlit","sec", 1, 60, 5, 1)
  , FIELD(settings.alarm_hour[1],"Start","hour", 0, 23, 1, 1)
  , FIELD(settings.alarm_min[1],"Start","min", 0, 59, 10, 1)
  , OP("Exit", quit)
);

MENU(mainMenu,"Main"
  , SUBMENU(timer1_menu)
  , SUBMENU(timer2_menu)
  , OP("Reset", reset_settings)
  , OP("Exit", close_settings_menu)
);

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
	
	load_settings();
	
	//debug ---> !!!
	/*
	settings.alarm_enable[0] = true;
	settings.alarm_type[0] = POLIV_TYPE;
	settings.alarm_hour[0] = 18;
	settings.alarm_min[0] = 55;
	settings.alarm_duration[0] = 10;
	
	settings.alarm_enable[1] = true;
	settings.alarm_type[1] = RELAY1_TYPE;
	settings.alarm_hour[1] = 18;
	settings.alarm_min[1] = 16;
	settings.alarm_duration[1] = 10;
	
	set_time();
	setup_alarms();*/
	//< --- debug !!!
	
	setSyncProvider(&sync_time);
    setSyncInterval(60); 
	
	start_screen();
}

// the loop routine runs over and over again forever:
void loop() 
{
	kran.poll();
	if (poll_menu)
	{
        logln("poll menu");
		mainMenu.poll(menu_lcd, allIn);
	}
	else if (allIn.available()) 
	{
        char ch = allIn.read();
		if (ch == menu::enterCode) 
		{
			poll_menu = true;
			mainMenu.sel = 0; //0 reset the menu index for next call
			logln("open menu");
		}
        else if (ch == menu::upCode) 
		{
			turn_kran(POLIV_MANUAL_LONG);
		}
    }
    else 
    {
        if (kran.opened())
        {
            int sec = 	kran.poliv_left_sec();
			sprintf (&buf__[0], "POLIV: %d         ", sec);
            print_screen(buf__, "STOP", nullptr);
            logln(buf__);
        }
        else
        {
            logln("no buttons");
            start_screen();
        }
		
	}				
  
	blink();
	Alarm.delay(1);        // delay in between reads for stability
}
