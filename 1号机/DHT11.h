#pragma once
#ifndef _DHT11_h
#define _DHT11_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


struct DHT11_data
{
  float temperature;
  float humidity;
};


//void my_delay_us(unsigned long timeout_us);
void DHT11_init(unsigned char pin1);
char read_ready();
char read_data(struct DHT11_data* value);

#endif
