#ifndef _USERMAIN_H
#define _USERMAIN_H
#include <ESP8266WiFi.h>
#include "savevalues.h"
#include "mywifi.h"
#include "myconstant.h"
extern "C"
{
#include "DHT11.h"
#include "mytimer.h"
//必须实现的
    void my_init();
    void set_data_(short i, short value);
    void add_values();
    int set_databack(const char fig,char *tcp_send_data);
    void timer1_worker();
    //下面连个函数禁止执行长时间阻塞的函数
    void ruan_timer_ms();//每隔 RUAN_TIMEer_ms
    void ruan_timer_us();//每隔 RUAN_TIMEer_us
    
    //str_data_names 的长度
    #define MAX_NAME 13 
    extern const char *str_data_names[MAX_NAME];
    extern const char *MODE_INFO;
    extern uint8_t power_save;

//选择实现的

    uint8_t light_high();
    uint8_t sheng_yin_high();
    uint8_t shengyin_and_light_high();
    void shengyin_timeout_out();
    void set_jdq(const uint8_t pin_x, short switch_x, uint8_t *LEDx);
    void brightness_work();
    void DHT11_read_and_send();
    void dht11_get();
    void read_dht11();
}
#endif