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


#define BUTTON_1 4   
#define BUTTON_2 5   

#define OPEN_PIN 6   //Открытие крана
#define CLOSE_PIN 7  //Закрытие крана
#define OPEN_TIME_MSEC 1000  //Время нужное для открытия/закрытия крана
#define LED_KRAN  3      //Горит - движется кран
#define BUTTON_START BUTTON_1   //Начать полив (открыть на N сек), закрыть кран

#define LED_WORK  2      //Моргает - отдых, горит - полив

#define BLINK_DELAY   2000   //Период моргания
#define BLINK_TIME    100   //Период моргания

unsigned long blink_time = 0;
bool blink_on = false;

Kran kran(OPEN_PIN, CLOSE_PIN, LED_KRAN, OPEN_TIME_MSEC);

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
menuLCD menu_lcd(lcd,16,2);//menu output device

//a keyboard with only one key :D, this is the encoder button
keyMap encBtn_map[]={{BUTTON_1,menu::upCode}, {BUTTON_2,menu::enterCode}};//negative pin numbers means we have a pull-up, this is on when low
keySafeLook<2> encButton(encBtn_map);
Stream* in3[]={&encButton,&Serial};
chainStream<2> allIn(in3);

bool poll_menu = false;

bool poliv_enable = true;
int poliv_run_hour = 0;
int poliv_run_min = 0;
int poliv_period = 24;
int poliv_duration = 3;

void turn_kran()
{
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
   kran.set_duration(poliv_duration*1000);
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
    Serial.println("start_menu");
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

bool start_screen()
{
	poll_menu = false;
    //button1.attachClick(turn_kran);
    update_duration();
    
    print_screen("Hello!", "POLIV", "MENU");
    
	Serial.println("start_screen");
	
	return true;
}

void reset_settings()
{

}

TOGGLE(poliv_enable,poliv,"POLIV: ",
  VALUE("YES",1),
  VALUE("NO",0)
);

MENU(mainMenu,"Main"
  , SUBMENU(poliv)
  , FIELD(poliv_period,"PERIOD","h", 8, 96, 8, 1)
  , FIELD(poliv_duration,"DLIT","sec", 1, 60, 5, 1)
  , FIELD(poliv_run_hour,"Start","hour", 0, 23, 1, 1)
  , FIELD(poliv_run_min,"Start","min", 0, 59, 10, 1)
  , OP("Exit", start_screen)
);

void setup()
{
  //int sensorValue = analogRead(RESISTOR_PIN);  //int (0 to 1023)
  kran.setup();
  
  pinMode(LED_WORK, OUTPUT);
  digitalWrite(LED_WORK, LOW);
  
  pinMode(BUTTON_1,INPUT);
  digitalWrite(BUTTON_1,LOW);
  pinMode(BUTTON_2,INPUT); 
  digitalWrite(BUTTON_2,LOW);
  
  lcd.begin(16,2);  
  Serial.begin(9600) ;
    
  //button2.attachClick(start_menu);
  start_screen();
}

// the loop routine runs over and over again forever:
void loop() 
{
	kran.poll();
	if (poll_menu)
	{
        Serial.println("poll menu");
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
			Serial.println("open menu");
		}
        else if (ch == menu::upCode) 
		{
			Serial.println("turn_kran");
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
               
            Serial.print("poliv left: ");
            Serial.println(left);
        }
        else
        {
            Serial.println("no buttons");
            start_screen();
        }
		
	}				
  
	blink();
	delay(1);        // delay in between reads for stability
}
