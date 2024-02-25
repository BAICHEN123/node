#ifndef _MYCONSTANT_h
#define _MYCONSTANT_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
//复位函数
//ESP.restart();

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
//心跳包回复时间上限，超时认为连接断裂，重新连接,
#define HEART_BEAT_TIMEOUT_ms HEART_BEAT_ms * 2 - 1

#define list_values_len_max 10 //需要存储多少个变量的值

#define MAX_TCP_DATA 1025      //TCP缓存的最大值
#define MAX_UDP_SEND_DATA 1024 //UDP缓存的最大值

//wifi 的名称密码长度限制
#define WIFI_SSID_LEN 32
#define WIFI_PASSWORD_LEN 32

//wifi 相关
//做服务器时监听的端口
#define SERVER_CLIENT_PROT 9997

//以下两个定时可以同时调用，间隔时间不是很准确，当发生网络请求的时候，RUAN_TIMEer_us 可能会变大
//软定时，主函数循环调用的间隔时间
#define RUAN_TIMEer_ms 10
//软定时，主函数循环调用的间隔时间
#define RUAN_TIMEer_us 100

#if RUAN_TIMEer_ms>TIMER1_timeout_ms
#error "软定时调用时间大于定时中断，会导致udp无法发送完成"
#endif

//心跳包使用 HEART_BEAT_FIG 开头，数据包采用 COMMAND_FIG 开头
#define HEART_BEAT_FIG '\t'
#define COMMAND_FIG '#'

/*一个时刻内，错误的最大长度
节点错误中会缓存监听量和手动设置的错误，这里设定的值应该大于两者之和最合适，但是内存可能会说不
*/
#define WARN_LEN 35
//一个时刻内，监听的最大数量
#define JianTin_MAX_LEN 30


extern u64 EID;
extern String UDP_head_data;
extern String UDP_send_data;
extern const char *wifi_ssid_pw_file; //储存 WiFi 账号和密码的文件
extern const char *stut_data_file;    //储存设备各功能配置状态的文件
extern const char *MYHOST;            //服务器 ip 地址


//str_data_names 的长度
#define MAX_NAME 40
//extern const char *str_data_names[MAX_NAME];
extern struct MyType data_list[MAX_NAME];
//可选变量描述
extern const char *MODE_INFO;
//断电记忆的可选参数
extern uint8_t power_save;


#endif
