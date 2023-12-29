#pragma once
#ifndef _DHT11_h
#define _DHT11_h
#include "arduino.h"

struct DHT11_data
{
  float temperature;
  float humidity;
};
#define LOW_PIN_ms 22

void dht11_init(unsigned char pin1);
char dht11_read_ready();
short dht11_read_data(struct DHT11_data* value);

#endif
