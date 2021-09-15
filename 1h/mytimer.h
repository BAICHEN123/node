#ifndef _MYTIMER_H
#define _MYTIMER_H
//#include <ESP8266WiFi.h>
#include <Arduino.h>

void set_timer1_s(timercallback userFunc, double time_s);
void set_timer1_ms(timercallback userFunc, uint32_t time_ms);
/*微秒定时器 定时中断函数里禁止调用 delay 进行延时操作，调用必暴毙
userFunc 需要定时调用执行的函数名*/
void set_timer1_us(timercallback userFunc, uint32_t time_us);

#endif