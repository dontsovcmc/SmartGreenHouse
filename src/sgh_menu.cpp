
#include "sgh_menu.h"
#include "sgh_lcd.h"
#include "pins.h"

#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd;

// Menu data
menuLCD menu_lcd(lcd,16,2);
keyMap encBtn_map[]={{BUTTON_1,menu::upCode}, {BUTTON_2,menu::enterCode}};//negative pin numbers means we have a pull-up, this is on when low
keyLook<2> encButton(encBtn_map);
Stream* in2[]={&encButton,&Serial};
chainStream<2> allIn(in2);



