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

#include "multi_desktop.h"

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

void turn_on_relay1(int duration)
{
	logln("relay1");
	digitalWrite(RELAY1_PIN, !digitalRead(RELAY1_PIN));
}

void turn_on_relay2(int duration)
{
	logln("relay2");
	digitalWrite(RELAY2_PIN, !digitalRead(RELAY1_PIN));
}


//void alarm0() { alarm_function(0); }

//alarmFunction alarm_func[ALARMS] = { &alarm0, &alarm1, &alarm2, &alarm3, &alarm4};


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
	screen_info("Settings\nreset OK", 500);
	return false;
}

bool close_settings_menu()
{
	screen_info("Settings saved", 500);
	
	start_screen();
	return true;
}


MENU(mainMenu,"Main"
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



void turn()
{
	turn_kran(POLIV_MANUAL_LONG);
}

void relay1()
{
	turn_on_relay1(2);
}

void menu_button()
{
	screen_info("MENU", 1000);
}

void show_1_screen()
{
	//screen_info("DESK 1", 1);
	get_rtc_time_str(buf__, BUF_LEN);
	print_screen(buf__, "POLIV", "D 2");
			
}

void show_2_screen()
{
	print_screen("DESK 2", "RELAY 1", "D 3");
}

void show_3_screen()
{
	print_screen("DESK 3", "MENU", "D 1");
}

Desktop mainDesktop(&show_1_screen, &turn); 
Desktop secondDesktop(&show_2_screen, &relay1);
Desktop thirdDesktop(&show_3_screen, &menu_button);

MultiDesktop multi_desktop(&mainDesktop);



void setup()
{
	lcd.begin(16,2);  
	Serial.begin(9600);
	
	screen_info("HELLO", 500);
	
	multi_desktop.add_desktop(&secondDesktop);
	multi_desktop.add_desktop(&thirdDesktop);
}

void loop() 
{
	kran.poll();
	
	if (allIn.available()) 
	{
        char ch = allIn.read();
		
		multi_desktop.button_pressed(ch);
    }
	
	multi_desktop.show();
	Alarm.delay(1); // delay in between reads for stability
}
