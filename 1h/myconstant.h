//#pragma once
#ifndef _MYCONSTANT_h
#define _MYCONSTANT_h

//#include <Arduino.h>
//#include <ESP8266WiFi.h>
//复位函数
void resetFunc();

//定时器1 的单次中断时长
#define TIMER1_timeout_ms 200

//DHT11读取数据时间间隔
#define DHT11_SPACE_OF_TIME_ms 1500
#define TIMER2_COUNT DHT11_SPACE_OF_TIME_ms / TIMER1_timeout_ms

//清除wifi信息的按键按下时间
#define CLEAR_WIFI_DATA_S 5
#define CLEAR_WIFI_DATA_COUNT CLEAR_WIFI_DATA_S * 1000 / TIMER1_timeout_ms

//心跳包时间间隔
#define HEART_BEAT_ms 60 * 1000

#define list_values_len_max 10 //需要存储多少个变量的值

#define MAX_TCP_DATA 1024      //TCP缓存的最大值
#define MAX_UDP_SEND_DATA 1024 //UDP缓存的最大值

//wifi 的名称密码长度限制
#define WIFI_SSID_LEN 32
#define WIFI_PASSWORD_LEN 32

//wifi 相关
//做服务器时监听的端口
#define SERVER_CLIENT_PROT 9997

#endif
