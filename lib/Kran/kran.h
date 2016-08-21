#ifndef Kran_h
#define Kran_h

#include "Arduino.h"

class Kran
{
public:
    Kran(int open_pin_, int close_pin_, int led_kran_, unsigned long open_time_msec_);
    void setup();
    void poll();
    
    bool opened();
    unsigned long poliv_left_sec();
    unsigned long get_duration();
    void set_duration(unsigned long d);
    void open_kran();
    void open_kran_stop();
    void close_kran();
    void close_kran_stop();
    void start_work();
    void stop_work();

private:
    
    unsigned long start_work_time;
  
    unsigned long start_open_time;
    unsigned long start_close_time;

    int open_pin;
    int close_pin;
    int led_kran;
    unsigned long open_time_msec;
	unsigned long duration;
};

#endif
