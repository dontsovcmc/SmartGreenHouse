
#include "kran.h"
    
Kran::Kran(int open_pin_, int close_pin_, int led_kran_, unsigned long open_time_msec_)
  : open_pin(open_pin_)
  , close_pin(close_pin_)
  , led_kran(led_kran_)
  , open_time_msec(open_time_msec_)
  , start_work_time(0)
  , start_open_time(0)
  , start_close_time(0)
{
}

void Kran::setup()
{
    pinMode(open_pin, OUTPUT);
    pinMode(close_pin, OUTPUT);

    pinMode(led_kran, OUTPUT);

    digitalWrite(open_pin, LOW);
    digitalWrite(close_pin, LOW);
    digitalWrite(led_kran, LOW);
}

void Kran::poll()
{
    if (start_open_time > 0)
    {
       if (millis() - start_open_time > open_time_msec)
           open_kran_stop();
    }

    if (start_close_time > 0)
    {
       if (millis() - start_close_time > open_time_msec)
           close_kran_stop();
    }

    if (start_work_time > 0)
    {
       unsigned long duration = get_duration();
       if (millis() - start_work_time > duration)
           stop_work();
    }
}

bool Kran::opened()
{
   return start_work_time > 0;
}

unsigned long Kran::get_duration()
{
    //unsigned long d = analogRead(A0); // 0 - 1023
    //d *= 1000;   // 0 - 1023 seconds
    return duration;
}

unsigned long Kran::poliv_left_sec()
{
   return (duration - (millis() + start_work_time))/ 1000;
}

void Kran::set_duration(unsigned long d)
{ 
	duration = d; 
}
 
void Kran::open_kran()
{
    start_open_time = millis();
    digitalWrite(open_pin, HIGH);
    digitalWrite(led_kran, HIGH);
}

void Kran::open_kran_stop()
{
    start_open_time = 0;
    digitalWrite(open_pin, LOW);
    digitalWrite(led_kran, LOW);
}

void Kran::close_kran()
{
    start_close_time = millis();
    digitalWrite(close_pin, HIGH);
    digitalWrite(led_kran, HIGH);
}

void Kran::close_kran_stop()
{
    start_close_time = 0;
    digitalWrite(close_pin, LOW);
    digitalWrite(led_kran, LOW);
}

void Kran::start_work()
{
    start_work_time = millis();
    open_kran();
}

void Kran::stop_work()
{
    start_work_time = 0;
    close_kran();
}
