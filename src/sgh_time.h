
#ifndef SGH_TIME_H
#define SGH_TIME_H

#include <stdio.h>
#include <Wire.h>
#include "RTClib.h"

void get_time_str(RTC_DS1307 *rtc, char *buf, const int len)
{
	DateTime now = rtc->now();
    sprintf (buf, "%d/%d/%d %d:%d", now.year(), now.month(), now.day(), now.hour(), now.minute());
}

#endif
