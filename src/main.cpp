#include "Arduino.h"
#include "stdlib.h"

#define ONLY_UP_KEY

#include <menu.h>
#include <menuLCDs.h>
#include <menuFields.h>
#include <keyStream.h>
#include <chainStream.h>// concatenate multiple input streams (this allows adding a button to the encoder)

#include <TimeAlarms.h>

#include <multidesktop.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include "kran.h"

#define SOFTWARE_VER "1.7"

#include "pins.h"
#include "settings.h"
#include "sgh_time.h"
#include "sgh_lcd.h"
#include "sgh_menu.h"


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

char alarms_id[ALARMS]; 
uint8_t show_alarm_index;  //0 - ALARMS

Kran kran(OPEN_PIN, CLOSE_PIN, LED_KRAN, OPEN_TIME_MSEC);

// Menu data
extern menuLCD menu_lcd;
extern chainStream<2> allIn;

#define BUF_LEN 32
char buf__[BUF_LEN];  //for printf 

unsigned long blink_time = 0;
bool blink_on = false;
time_t relay1_start_point = 0;
bool poll_menu = false;


extern "C" {
  typedef void (*alarmFunction)(void);
}

/**
@brief screen_info

       print message on LCD screen and delay some time
*/
void screen_info(const char *buffer, int delay)
{
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
    LOG("relay1 OFF");
    digitalWrite(RELAY1_PIN, LOW);    
}

void turn_off_relay2()
{
    LOG("relay2 OFF");
    digitalWrite(RELAY2_PIN, LOW);    
}

void turn_on_relay1(int duration)
{
    LOG("relay1 ON");
    digitalWrite(RELAY1_PIN, HIGH);
    Alarm.timerOnce(duration, turn_off_relay1); 
}

void turn_on_relay2(int duration)
{
    LOG("relay2 ON");
    digitalWrite(RELAY2_PIN, HIGH);
    Alarm.timerOnce(duration, turn_off_relay2); 
}

/**
@brief alarm_function

       run alarm by it settings id (0 - ALARMS)
*/
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

/**
@brief blink

       blinking green led
*/
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


/**
@brief setup_alarms

       setup alarms that configured in settings or update alarms
*/
void setup_alarms()
{
    for (int i=0; i < ALARMS; i++)
    {
        if (settings.alarms[i].alarm_enable)
        {
            if (alarms_id[i] == -1)
            {
                alarms_id[i] = Alarm.alarmRepeat(settings.alarms[i].alarm_hour, settings.alarms[i].alarm_min, 0, alarm_func[i]);
                snprintf (buf__, BUF_LEN, "ALARM %d: %02d:%02d", i+1, settings.alarms[i].alarm_hour, settings.alarms[i].alarm_min);
            }
            else
            {
                Alarm.write(alarms_id[i], AlarmHMS(settings.alarms[i].alarm_hour, settings.alarms[i].alarm_min, 0));
                snprintf (buf__, BUF_LEN, "UPDATE %d: %02d:%02d", i+1, settings.alarms[i].alarm_hour, settings.alarms[i].alarm_min);
            }
            
            screen_info(buf__, 500);
        }
    }
}

/**
@brief load_settings

       read settings from memory,
       read time from RTC 
       setup alarms that configured in settings
*/
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

/**
@brief Main screen print function. 
       Press Button1: start poliv
       Press Button2: next screen
*/
void main_screen()
{
    if (kran.opened())
    {
        int sec =     kran.poliv_left_sec();
        sprintf (&buf__[0], "POLIV: %d         ", sec);
        print_screen(buf__, "STOP", nullptr);
    }
    else
    {
		sensors.requestTemperatures();
		float t = sensors.getTempCByIndex(0);
		
		char time_s[16], t_s[5];
		dtostrf(t, 4, 1, t_s);
		
        get_rtc_time_str(time_s, BUF_LEN);
		
		if (t > 0.0)
			snprintf (buf__, BUF_LEN, "%s +%sC", time_s, t_s);  //manual print message and buttons
		else
			snprintf (buf__, BUF_LEN, "%s %sC", time_s, t_s);
		
        print_screen(buf__, "POLIV", ">rele");
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


/**
@brief Alarm screen print function. 
       Press Button1: show next configured alarm
       Press Button2: next screen
*/
void alarm_screen()
{
	while (!settings.alarms[show_alarm_index].alarm_enable && show_alarm_index < ALARMS)
        show_alarm_index++;
	if (show_alarm_index != ALARMS)
	{
		time_t elapsed = Alarm.getNextTriggerById(alarms_id[show_alarm_index]) - now();

		const char *atypes[] = {"Poliv", "Rele1", "Rele2"};

		snprintf (buf__, BUF_LEN, "%d:%s-%02d:%02d:%02d", show_alarm_index, 
			atypes[settings.alarms[show_alarm_index].alarm_type], 
			hour(elapsed), minute(elapsed), second(elapsed));
	}
	else
	{
		snprintf (buf__, BUF_LEN, "no alarms");
	}	
    
    print_screen(buf__, ">>", ">menu");
}

void alarm_screen_action()
{
    show_alarm_index++;
    while (!settings.alarms[show_alarm_index].alarm_enable && show_alarm_index < ALARMS)
        show_alarm_index++;
    
    if (show_alarm_index >= ALARMS)
        show_alarm_index = 0;
}

/**
@brief Rele screen print function. 
       Press Button1: Toggle Rele 1
       Press Button2: next screen
*/
void relay_screen()
{    
    if (digitalRead(RELAY1_PIN))  //on
    {
        if (relay1_start_point > 0)
        {
            time_t t = now() - relay1_start_point + 1;
            snprintf (buf__, BUF_LEN, "Rele1: %02d:%02d:%02d", 
                hour(t), minute(t), second(t));
        }
        else
        {
            relay1_start_point = now();
        }
        
        print_screen(buf__, "OFF", ">alarms");
    }
    else
    {
        relay1_start_point = 0;
        print_screen("Rele1", "ON", ">alarms");
    }
}

void relay_screen_action() { digitalWrite(RELAY1_PIN, !digitalRead(RELAY1_PIN)); }

/**
@brief  reset_settings 

        reset settings 
*/
bool reset_settings()
{
    init_settings();
    write_settings();
    screen_info("Settings\nreset OK", 500);
    return false;
}

/**
@brief  close_settings_menu
		
		close menu
		save settings to memory
		load settings from memory 
		
*/
bool close_settings_menu()
{
    poll_menu = false;
    
    write_settings();
    screen_info("Settings saved", 500);
    load_settings();
    
    return true;
}

/**
@brief ArduinoMenu library 
*/
promptFeedback cancel() { return true; }
promptFeedback before_alarm_submenu();
promptFeedback exit_alarm_submenu();

// tempopary 1 alarm settings for submenu.
alarm_settings alarm_sett; 

TOGGLE(alarm_sett.alarm_enable,alarm_submenu,"Work: ",
  VALUE("YES",1),
  VALUE("NO",0)
);

TOGGLE(alarm_sett.alarm_type,alarm_type_menu,"Type: ",
  VALUE("Poliv",POLIV_TYPE),
  VALUE("Rele 1",RELAY1_TYPE),
  VALUE("Rele 2",RELAY2_TYPE)
);

MENU(alarm_menu,"Alarm def"
  , SUBMENU(alarm_submenu)
  , SUBMENU(alarm_type_menu)
  , FIELD(alarm_sett.alarm_duration,"Dlit","sec", 1, 60, 5, 1)
  , FIELD(alarm_sett.alarm_hour,"Start","hour", 0, 23, 1, 1)
  , FIELD(alarm_sett.alarm_min,"Start","min", 0, 59, 10, 1)
  , OP("OK", exit_alarm_submenu)
);

MENU(mainMenu,"Main"
  , OP("Alarm 1",before_alarm_submenu)
  , OP("Alarm 2",before_alarm_submenu)
  , OP("Alarm 3",before_alarm_submenu)
  , OP("Alarm 4",before_alarm_submenu)
  , OP("Alarm 5",before_alarm_submenu)
  , OP("Reset", reset_settings)
  , OP("Exit", close_settings_menu)
);

/**
@brief before_alarm_submenu enter Alarm submenu to load alarm from settings
*/
promptFeedback before_alarm_submenu() {
    alarm_menu.sel = 0;
    alarm_sett = settings.alarms[mainMenu.sel];
    menuNode::activeNode = &alarm_menu;
    return false;
}

/**
@brief exit_alarm_submenu exit from Alarm submenu to save alarm to settings
*/
promptFeedback exit_alarm_submenu() {
    settings.alarms[mainMenu.sel] = alarm_sett;
    return cancel();
}

/**
@brief Menu screen print function. 
       Press Button1: Open settings menu (ArduinoMenu)
       Press Button2: next screen
*/
void menu_screen()
{
    print_screen("show menu", "MENU", ">poliv");
}

void menu_screen_action()
{
    poll_menu = true;
    mainMenu.sel = 0; //0 reset the menu index for next call
    LOG("open menu");
}

/* Multi Desktop configuration */
#define BUTTONS 1  //number of action buttons
int8_t codes[] = { menu::upCode }; //from ArduinoMenu library defines

func desktop1cb[] = { &start_screen_action };
func desktop2cb[] = { &relay_screen_action };
func desktop3cb[] = { &alarm_screen_action };
func desktop4cb[] = { &menu_screen_action };

/* "Multi desktop menu" you can change Button1 action by pressing Button2 */
MultiDesktop<BUTTONS> multi_desktop( menu::enterCode , codes);
Desktop<BUTTONS> mainScreen(&main_screen, desktop1cb); 
Desktop<BUTTONS> relayScreen(&relay_screen, desktop2cb);
Desktop<BUTTONS> alarmScreen(&alarm_screen, desktop3cb);
Desktop<BUTTONS> menuScreen(&menu_screen, desktop4cb);


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
    show_alarm_index = 0;
    
    for (int i=0; i < ALARMS; i++, alarms_id[i] = -1) ;
    
	sensors.begin();
    kran.setup();
    pin_setup();
    
    lcd.begin(16,2);  
    Serial.begin(9600);

    load_time(buf__, BUF_LEN);
    screen_info(buf__, 1000);
    
    
    snprintf (buf__, BUF_LEN, "VERSION %s", SOFTWARE_VER);
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
    
    multi_desktop.add_desktop(&mainScreen);
    multi_desktop.add_desktop(&relayScreen);
    multi_desktop.add_desktop(&alarmScreen);
    multi_desktop.add_desktop(&menuScreen);
    
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
