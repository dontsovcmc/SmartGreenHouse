/**************
Sept. 2016 Dontsov Evgeny
creative commons license 3.0: Attribution-ShareAlike CC BY-SA
This software is furnished "as is", without technical support, and with no
warranty, express or implied, as to its usefulness for any purpose.

Thread Safe: No
Extendable: Yes

quick and dirty keyboard driver
metaprog keyboard driver where N is the number of keys
all keys are expected to be a pin (buttons)
we can have reverse logic (pull-ups) by entering negative pin numbers
ex: -A0 means: pin A0 normally high, low when button pushed (reverse logic)

this driver is for simple buttons (jitter safery)
you can redefine BOUNCE_TICK and LONG_PRESS_TICK in your code
***/

#include "keyStream.h"

#ifndef BOUNCE_TICK
#define BOUNCE_TICK 30
#endif

#ifndef LONG_PRESS_TICK
#define LONG_PRESS_TICK  700
#endif

template <int N>
class keySafeLook:public Stream {
public:
  keyMap* keys;
  int lastkey;
  unsigned long pressMills=0;
  
  keySafeLook<N>(keyMap k[])
    : keys(k)
    , lastkey(-1) 
    {}
    
  int available(void) {
    int ch=peek();
    
    if (lastkey==-1) 
    {
      lastkey=ch;
      pressMills=millis();
    } 
    else 
    {   
        if (ch == -1 && millis()-pressMills < BOUNCE_TICK) {
            lastkey = -1;  //released = it's bounce. reset lastkey
            return 0;
        }   
        return 1;
    }
    if (ch==lastkey) 
        return 0;
    return 1;
  }
  int peek(void) {
    for(int n=0;n<N;n++) {
      int8_t pin=keys[n].pin;
      if (digitalRead(pin<0?-pin:pin)!=(pin<0) ) return keys[n].code;
    }
    return -1;
  }
  int read() {
    int ch=peek();
    if (ch==lastkey) return -1;
        
    int tmp=lastkey;
    bool longPress = (unsigned long)(millis() - pressMills) > LONG_PRESS_TICK;
    pressMills=millis();
    lastkey=ch;
    return longPress?menu::escCode:tmp;
  }
  void flush() {}
  size_t write(uint8_t v) {return 0;}
};
