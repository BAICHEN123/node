#pragma once
#ifndef _MYLIGHT_h
#define _MYLIGHT_h

//定时器1 的单次中断时长
#define TIMER1_timeout_ms 200

//DHT11读取数据时间间隔
#define DHT11_SPACE_OF_TIME_ms 1500
#define TIMER2_COUNT DHT11_SPACE_OF_TIME_ms / TIMER1_timeout_ms

//清除wifi信息的按键按下时间
#define CLEAR_WIFI_DATA_S 5
#define CLEAR_WIFI_DATA_COUNT CLEAR_WIFI_DATA_S * 1000 / TIMER1_timeout_ms

//心跳包时间间隔
#define HEART_BEAT_ms 10 * 1000
#endif