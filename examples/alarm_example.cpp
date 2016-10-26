#include "Arduino.h"
#include "stdlib.h"
#include "kran.h"
#include "wire.h"
#include <LiquidCrystal_I2C.h>
#include <menu.h>//menu macros and objects
#include <menuLCDs.h>//F. Malpartida LCD's
#include <menuFields.h>
#include <keySafeStream.h>
#include <chainStream.h>// concatenate multiple input streams (this allows adding a button to the encoder)

#include <TimeAlarms.h>

char poliv_alarm_id = -1;
char relay1_alarm_id = -1;

#include "settings.h"
#include "sgh_time.h"

#define BUTTON_1 4   
#define BUTTON_2 5   

#define RELAY1_PIN 8 //Реле 1
#define RELAY2_PIN 9 //Реле 2

#define OPEN_PIN 6   //Открытие крана
#define CLOSE_PIN 7  //Закрытие крана

#define OPEN_TIME_MSEC 1000  //Время нужное для открытия/закрытия крана
#define LED_KRAN  3      //Горит - движется кран
#define BUTTON_START BUTTON_1   //Начать полив (открыть на N сек), закрыть кран

Kran kran(OPEN_PIN, CLOSE_PIN, LED_KRAN, OPEN_TIME_MSEC);

#define LED_WORK  2      //Моргает - отдых, горит - полив

#define BLINK_DELAY   2000   //Период моргания
#define BLINK_TIME    100   //Период моргания


unsigned long blink_time = 0;
bool blink_on = false;


LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
menuLCD menu_lcd(lcd,16,2);//menu output device


//a keyboard with only one key :D, this is the encoder button
keyMap encBtn_map[]={{BUTTON_1,menu::upCode}, {BUTTON_2,menu::enterCode}};//negative pin numbers means we have a pull-up, this is on when low
keySafeLook<2> encButton(encBtn_map);
Stream* in3[]={&encButton,&Serial};
chainStream<2> allIn(in3);

bool poll_menu = false;
bool relay1 = false;

void log(const char *str) { Serial.print(str); }
void logln(const char *str) { Serial.println(str); }


void turn_kran()
{
	logln("turn_kran");
    if (kran.opened())
    {
       lcd.setCursor(0,0); 
       lcd.print("POLIV STOPPED");
       kran.stop_work();
    }
    else
    {
       lcd.setCursor(0,0); 
       lcd.print("start");
       kran.start_work();
    }
}

void update_duration()
{
   kran.set_duration(settings.poliv_duration*1000);
}

void turn_off_fan()
{
	logln("FAN OFF");
	digitalWrite(RELAY1_PIN, LOW);	
	relay1 = false;
}

void turn_on_fan()
{
	logln("FAN ON");
    Alarm.timerOnce(settings.fan_duration, turn_off_fan); 
	digitalWrite(RELAY1_PIN, HIGH);
	relay1 = true;
}


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
}

void screen_info(const char *buffer, int delay)
{
	print_screen(buffer, nullptr, nullptr);
	Alarm.delay(delay);
	lcd.clear();		
}

void setup_settings()
{
	char ver = load_settings();
	if (ver > 0)
	{
		char ver_str[] = "SETT. LOADED   ";
		itoa(ver,&ver_str[13],10); //(integer, yourBuffer, base
		screen_info(ver_str, 1500);
	}
	else
	{
		screen_info("SETT. ERROR", 1500);
	}
	lcd.clear();
}


void load_time()
{
	bool parse = false;
	bool config = false;
	if (!RTC.isRunning()) 
	{
		tmElements_t tm;
		
		if (getDate(&tm, __DATE__) && getTime(&tm, __TIME__)) 
		{
			parse = true;
			// and configure the RTC with this info
			if (RTC.write(tm)) {
				config = true;
			}
		}
		
		if (parse && config) 
			screen_info("DS1307 configured", 1500);
		else if (parse) 
			screen_info("DS1307 Error", 1500);
		else 
			screen_info("No PC time", 1500);
	}
	else
	{
		char buffer[32];
		get_time_str(&buffer[0], 16);
		screen_info(buffer, 1500);
	}
}


void setup_alarms()
{
	
	if (settings.poliv_enable)
	{
		if (poliv_alarm_id == -1)
			poliv_alarm_id = Alarm.alarmRepeat(settings.poliv_run_hour, settings.poliv_run_min, 0, turn_kran);   // every day		
		else
			Alarm.write(poliv_alarm_id, AlarmHMS(settings.poliv_run_hour, settings.poliv_run_min, 0));
		
		char buf[16];
		sprintf (buf, "POLIV: %02d:%02d", settings.poliv_run_hour, settings.poliv_run_min);
		screen_info(buf, 1500);
	}
	if (settings.fan_enable)
	{
		if (relay1_alarm_id == -1)
			relay1_alarm_id = Alarm.alarmRepeat(settings.fan_run_hour, settings.fan_run_min, 0, turn_on_fan); // every day	
		else
			Alarm.write(relay1_alarm_id, AlarmHMS(settings.fan_run_hour, settings.fan_run_min, 0));
		
		char buf[16];
		sprintf (buf, "RELAY1: %02d:%02d", settings.fan_run_hour, settings.fan_run_min);
		screen_info(buf, 1500);
	}
}
	

bool start_screen()
{
	char buffer[32];
	
	poll_menu = false;
    update_duration();
	
	if (second() / 2 % 2)
	{
		get_time_str(&buffer[0], 16);	
	}
	else
	{
		float t = 1.1;
		char t_s[6];
		dtostrf(t, 4, 1, t_s);
		sprintf (&buffer[0], "T = %s C          ", t_s);
	}
	
	print_screen(buffer, "POLIV", "MENU");
    
	logln("start_screen");
	
	return true;
}

bool reset_settings()
{
	init_settings();
	save_settings();
	
	setup_alarms();
	return false;
}

bool close_settings_menu()
{
	save_settings();
	screen_info("Settings saved", 1500);

	setup_internal_time();
	start_screen();
}

promptFeedback quit() {
  return true;
}

TOGGLE(settings.poliv_enable,poliv_on_menu,"WORK: ",
  VALUE("YES",1),
  VALUE("NO",0)
);
MENU(poliv_menu,"POLIV"
  , SUBMENU(poliv_on_menu)
  , FIELD(settings.poliv_duration,"DLIT","sec", 1, 60, 5, 1)
  , FIELD(settings.poliv_run_hour,"Start","hour", 0, 23, 1, 1)
  , FIELD(settings.poliv_run_min,"Start","min", 0, 59, 10, 1)
  , OP("Exit", quit)
);

TOGGLE(settings.fan_enable,fan_on_menu,"WORK: ",
  VALUE("YES",1),
  VALUE("NO",0)
);

MENU(fan_menu,"FAN"
  , SUBMENU(fan_on_menu)
  , FIELD(settings.fan_duration,"DLIT","sec", 1, 60, 5, 1)
  , FIELD(settings.fan_run_hour,"Start","hour", 0, 23, 1, 1)
  , FIELD(settings.fan_run_min,"Start","min", 0, 59, 10, 1)
  , OP("Exit", quit)
);


MENU(mainMenu,"Main"
  , SUBMENU(poliv_menu)
  , SUBMENU(fan_menu)
  , OP("Reset", reset_settings)
  , OP("Exit", close_settings_menu)
);


void setup()
{
	kran.setup();

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
	
	lcd.begin(16,2);  
	Serial.begin(9600) ;

	setup_settings();
	
	
	settings.poliv_enable = true;
	settings.poliv_run_hour = 18;
	settings.poliv_run_min = 26;
	settings.poliv_duration = 10;
	
	settings.fan_enable = true;
	settings.fan_run_hour = 18;
	settings.fan_run_min = 27;
	settings.fan_duration = 5;
	
	load_time();
	
	setup_alarms();
	
	start_screen();
}

// the loop routine runs over and over again forever:
void loop() 
{
	kran.poll();
	if (poll_menu)
	{
        logln("poll menu");
		mainMenu.poll(menu_lcd,allIn);
	}
	else if (allIn.available()) 
	{
		//button1.tick();
        char ch = allIn.read();
		if (ch == menu::enterCode) 
		{
			poll_menu = true;
			mainMenu.sel = 0; //0 reset the menu index fornext call
			logln("open menu");
		}
        else if (ch == menu::upCode) 
		{
			turn_kran();
		}
    }
    else 
    {
        if (kran.opened())
        {
            int left = 	kran.poliv_left_sec();
            char buffer[] = "POLIV:         ";
            
            itoa(left,&buffer[7],10); //(integer, yourBuffer, base)
			
            print_screen(buffer, "STOP", nullptr);
               
            log(buffer);
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