#include "usermain.h"
extern "C"
{
	const char *MODE_INFO = "@断电记忆[0-2]:关闭，仅本次，所有";


	//定义传感器储存变量
	uint8_t power_save = 0; //断电记忆


	//定义几个引脚的功能
	const uint8_t anjian1 = 0; //按键1输入

	//定义几个范围定义变量
	unsigned char CONST1[3] = {0, 1, 2};

	struct MyType data_list[MAX_NAME] = {
		{"@断电记忆", NULL, TYPE_u8, sizeof(power_save), &power_save, CONST1, CONST1 + 2}

	};

	void timer1_worker()
	{
		//长按按键1清除wifi账号密码记录
		clear_wifi_data(wifi_ssid_pw_file); 
	}

	void my_init()
	{
		pinMode(anjian1, INPUT); //按键1

		set_timer1_ms(timer1_worker, TIMER1_timeout_ms); //初始化定时中断
	}

	//需要断电记忆的变量在这里添加
	void add_values()
	{
		//记录舵机的状态
		//add_value(&duoji_need,sizeof(duoji_need));
	}

	//每隔 RUAN_TIMEer_ms
	void ruan_timer_ms()
	{
	}

	//每隔 RUAN_TIMEer_us
	void ruan_timer_us()
	{
	}

	//刷新状态
	void refresh_work()
	{
	}

}
