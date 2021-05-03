//#pragma once
#ifndef _MYCONSTANT_h
#define _MYCONSTANT_h

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



const char *wifi_ssid_pw_file = "/wifidata.txt";//储存 WiFi 账号和密码的文件
const char *stut_data_file = "/stutdata.txt";//储存设备各功能配置状态的文件
const char *MYHOST = "121.89.243.207";//服务器 ip 地址
const uint16_t TCP_PORT = 9999;
const uint16_t UDP_PORT = 9998;

//两个开关，当他为2时，是自动模式，其他时候读取12 和14号脚的电平
uint8_t LED1 = 0;
uint8_t switch_1 = 2;
uint8_t LED2 = 0;
uint8_t switch_2 = 2;
short switch_light_up_TIME_s = 30;	//重新加载的值//声控灯开启时长
short switch_light_up_time_x_s = 0; //计数器用
short TEMPERATURE_ERROR_HIGH = 40;
short TEMPERATURE_ERROR_LOW = 10;
uint8_t light_qu_yu = 5; //补光区间
uint8_t power_save = 0;	 //补光区间


#endif



