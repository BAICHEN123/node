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
#include "mytype.h"
//必须实现的
	/*初始化函数，初始化此项目需要使用的引脚*/
	void my_init();

	/*在函数内设置需要在断电保存的变量
	变量会在 power_save > 0 && 发生变动 的时候保存到 flash 
	一个变量变动，所有的变量都会保存一次*/
	void add_values();

	void ruan_timer_ms();//每隔 RUAN_TIMEer_ms 调用一次
	void ruan_timer_us();//每隔 RUAN_TIMEer_us 调用一次
    void ruan_timer_1s();//每隔 1s
	
	//刷新数据状态。my_init 初始化之后调用一次，用户每次对数据修改之后调用
	void refresh_work();
		
	//str_data_names 的长度
	#define MAX_NAME 20
	//extern const char *str_data_names[MAX_NAME];
	extern struct MyType data_list[MAX_NAME];
	//可选变量描述
	extern const char *MODE_INFO;
	//断电记忆的可选参数
	extern uint8_t power_save;

	/*注:
	记得在定时中断里执行 clear_wifi_data(wifi_ssid_pw_file); 这个指令,不然按按键不会清除已经记忆的wifi账号和密码
	*/



//选择实现的


	void dj_init();
	void dj_set_end();
	void dj_set();

}
#endif