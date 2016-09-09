#include "Arduino.h"

#include <menu.h>         // menu macros and objects
#include <keyStream.h>    // keyboard driver and fake stream (for the encoder button)
#include <chainStream.h>  // concatenate multiple input streams (this allows adding a button to the encoder)

#define BUTTON_1 4   
#define BUTTON_2 5   

keyMap encBtn_map[] = {{BUTTON_1, menu::upCode}};
keyLook<1> encButton(encBtn_map);
Stream* in1[] = {&encButton};
chainStream<1> allIn(in1);

void setup()
{
  Serial.begin(9600) ;
}

// the loop routine runs over and over again forever:
void loop() 
{
	if (allIn.available()) 
	{
		//button1.tick();
        char ch = allIn.read();
		if (ch == menu::enterCode) 
		{
			Serial.println("enterCode");
		}
        else if (ch == menu::upCode) 
		{
			Serial.println("upCode");
		}
    }
	delay(1);        // delay in between reads for stability
}
