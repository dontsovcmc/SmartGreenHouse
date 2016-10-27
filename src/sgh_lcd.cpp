
#include "sgh_lcd.h"

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);


void print_message(const char *buf)
{
	char *line2 = strchr(buf, '\n');
	if (line2) *line2 = '\0';
	
	lcd.setCursor(0, 0); 
	lcd.print(buf);
	for (unsigned int i = 0; SCREEN_WIDTH >=  strlen(buf) && i < SCREEN_WIDTH - strlen(buf); i++)
		lcd.print(" "); 
	
	if (line2) 
	{ 
		lcd.setCursor(0, 1);
		lcd.print(++line2);
		for (unsigned int i = 0; SCREEN_WIDTH >= strlen(line2) && i < SCREEN_WIDTH - strlen(line2); i++) 
		{
			lcd.print(" "); 
		}
	}
}

void print_screen(const char *message, const char *button1, const char *button2)
{
    print_message(message);
	
	lcd.setCursor(0,1); 
	int j = 0;
    if (button1)
    {
        lcd.print("[");
        lcd.print(button1);
        lcd.print("]");
		
		j += strlen(button1) + 2;
    }
    
	if (button2)
		j += strlen(button2) + 2;
	
    for (unsigned int i = 0; j <= SCREEN_WIDTH && i < SCREEN_WIDTH - j; i++)
        lcd.print(" ");        
    
    if (button2)
    {
        lcd.print("[");
        lcd.print(button2);
        lcd.print("]");
    }
}