#ifndef _USERMAIN_H
#define _USERMAIN_H
#include <ESP8266WiFi.h>
#include "savevalues.h"
#include "mywifi.h"
#include "myconstant.h"
#include "mywarn.h"
extern "C"
{
#include "mytimer.h"
//必须实现的
    /*初始化函数，初始化此项目需要使用的引脚*/
    void my_init();

    /*当节点收到指令，解析指令之后调用
    i       str_data_names 对应的索引
    value   需要设置的值
    */
    void set_data_(short i, short value);

    /*在函数内设置需要在断电保存的变量
    变量会在 power_save > 0 && 发生变动 的时候保存到 flash 
    一个变量变动，所有的变量都会保存一次*/
    void add_values();

    /*打包数据，将状态值和名称按协议打包好，存放在 tcp_send_data 里，主函数会在需要发送的时候调用这个函数*/
    int set_databack(const char fig,char *tcp_send_data);

    //下面2个函数禁止执行长时间任务
    void ruan_timer_ms();//每隔 RUAN_TIMEer_ms
    void ruan_timer_us();//每隔 RUAN_TIMEer_us
    
    //刷新数据状态，默认会在用户请求数据之前调用一次
    void refresh_work();
    
    //str_data_names 的长度
    #define MAX_NAME 6
    extern const char *str_data_names[MAX_NAME];
    //可选变量描述
    extern const char *MODE_INFO;
    //断电记忆的可选参数
    extern uint8_t power_save;

    /*注:
    记得在定时中断里执行 clear_wifi_data(wifi_ssid_pw_file); 这个指令,不然按按键不会清除已经记忆的wifi账号和密码
    */



//选择实现的



}
#endif