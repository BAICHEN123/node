#ifndef _MYTIMER_H
#define _MYTIMER_H
//#include <ESP8266WiFi.h>
#include <Arduino.h>

void set_timer1_s(timercallback userFunc, double time_s);
void set_timer1_ms(timercallback userFunc, uint32_t time_ms);

#endif