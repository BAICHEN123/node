#ifndef _MYDS18B20_h
#define _MYDS18B20_h
#include "arduino.h"

struct myds18b20
{
    /* data */
    unsigned char pin;
    void* obj;
    byte addr[8];
};


struct myds18b20 init_ds18b20(unsigned char pin1);

float read_temperature(struct myds18b20 dsdata);
void free_myds18b20(struct myds18b20 *dsdata);
#endif
