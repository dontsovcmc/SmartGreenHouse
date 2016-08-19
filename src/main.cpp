#include "Arduino.h"
#include "kran.h"
#include "OneButton.h"
#include "wire.h"
#include <LiquidCrystal_I2C.h>


#define BUTTON_1 4   
#define BUTTON_2 5   

#define OPEN_PIN 6   //Открытие крана
#define CLOSE_PIN 7  //Закрытие крана
#define OPEN_TIME_MSEC 3000  //Время нужное для открытия/закрытия крана
#define LED_KRAN  3      //Горит - движется кран
#define BUTTON_START BUTTON_1   //Начать полив (открыть на N сек), закрыть кран

#define RESISTOR_PIN A0  //Длительность открытия

#define LED_WORK  2      //Моргает - отдых, горит - полив

#define BLINK_DELAY   2000   //Период моргания
#define BLINK_TIME    100   //Период моргания

unsigned long blink_time = 0;
bool blink_on = false;

Kran kran(OPEN_PIN, CLOSE_PIN, LED_KRAN, OPEN_TIME_MSEC);
OneButton button1(BUTTON_1, false);
OneButton button2(BUTTON_2, false);

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

void turn_kran()
{
    if (kran.opened())
    {
       lcd.setCursor(0,1); 
       lcd.print("stop");
       kran.stop_work();
    }
    else
    {
       lcd.setCursor(0,1); 
       lcd.print("start");
       kran.start_work();
    }
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

void setup()
{
  //int sensorValue = analogRead(RESISTOR_PIN);  //int (0 to 1023)
  kran.setup();

  pinMode(LED_WORK, OUTPUT);
  digitalWrite(LED_WORK, LOW);
  
  lcd.begin(16,2);
  lcd.setCursor(0,0); 
  lcd.print("Hello, world!");
  
  Serial.begin(9600) ;
    
  button1.attachClick(turn_kran);
  button2.attachClick(main_menu);
}

void main_menu()
{

  lcd.setCursor(0,0); 
  lcd.print("menu");
}

// the loop routine runs over and over again forever:
void loop() {

  kran.poll();
  button1.tick();
  button2.tick();
  
  blink();

  delay(1);        // delay in between reads for stability
}
