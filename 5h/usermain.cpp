#include "usermain.h"
extern "C"
{
	const char *MODE_INFO = "@断电记忆[0-2]:关闭，仅本次，所有";

	uint8_t power_save = 0; //断电记忆

	//定义传感器储存变量
	double guang_qiang = 0; //光照强度
	uint8_t tong_feng = 0;	//通风扇的开关
	uint8_t duoji_need = 5; //舵机的期望值
	uint8_t duoji_now = 0;	//舵机的当前值

	//定义几个引脚的功能
	const uint8_t tongfeng = 14; //继电器
	const uint8_t duoji = 5;   //舵机
	const uint8_t anjian1 = 0; //按键1输入

	unsigned char CONST1[4] = {0, 1, 2, 5};

	struct MyType data_list[MAX_NAME] = {
		//{"红外人体检测", NULL, TYPE_u8, sizeof(hongwai), &hongwai,CONST1,CONST1+1},
		//{"烟雾逻辑", NULL, TYPE_u8, sizeof(yanwu), &yanwu,CONST1,CONST1+1},
		//{"烟雾模拟", "%", TYPE_DOUBLE, sizeof(yanwu_my), &yanwu_my,NULL,NULL},
		//{"@开关1", NULL, TYPE_u8, sizeof(LED1), &LED1,CONST1,CONST1+1},
		{"光强", "%", TYPE_DOUBLE, sizeof(guang_qiang), &guang_qiang, NULL, NULL},
		{"@舵机", NULL, TYPE_u8, sizeof(duoji_need), &duoji_need, CONST1 + 1, CONST1 + 3},
		{"@通风扇", NULL, TYPE_u8, sizeof(tong_feng), &tong_feng, CONST1, CONST1 + 1},
		{"@断电记忆", NULL, TYPE_u8, sizeof(power_save), &power_save, CONST1, CONST1 + 2}

	};

	void timer1_worker()
	{
		clear_wifi_data(wifi_ssid_pw_file); //长按按键1清除wifi账号密码记录
		guang_qiang = system_adc_read() * 100 / 1024.00;
		dj_set();
	}

	void my_init()
	{
		//pinMode(yan_wu, INPUT);
		pinMode(anjian1, INPUT); //按键1
		pinMode(tongfeng, OUTPUT);
		pinMode(duoji, OUTPUT);

		//初始化舵机的状态
		dj_init();
		set_timer1_ms(timer1_worker, TIMER1_timeout_ms); //强制重新初始化定时中断，如果单纯的使用 dht11_get 里的过程初始化，有概率初始化失败
	}

	void add_values()
	{
		//记录舵机的状态
		add_value(&duoji_need,sizeof(duoji_need));
		//记录通风扇的状态
		add_value(&tong_feng,sizeof(tong_feng));
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
		if(tong_feng==1)
		{
			digitalWrite(tongfeng, LOW);
		}
		else
		{
			digitalWrite(tongfeng, HIGH);

		}
	}




	
	//初始化舵机的位置
	void dj_init()
	{
		if (duoji_now != 0)
		{
			return;
		}
		char i = 0;
		duoji_now = duoji_need;
		for (i = 0; i < 5; i++)
		{
			digitalWrite(duoji, HIGH);
			unsigned long timeold = micros();
			while (micros() - timeold < duoji_need * 500)
			{
				_NOP();
			}
			digitalWrite(duoji, LOW);
			timeold = micros();
			while (micros() - timeold < (6 - duoji_need) * 500)
			{
				_NOP();
			}
			delay(17);
		}
	}

	void dj_set_end()
	{
		set_timer1_us(timer1_worker, TIMER1_timeout_ms * 1000 - duoji_now * 500); //正常时间之后恢复 timer1_worker 的工作
		digitalWrite(duoji, LOW);
	}
	
	void dj_set()
	{
		if (duoji_now > duoji_need)
		{
			duoji_now = duoji_now - 1;
			digitalWrite(duoji, HIGH);
			set_timer1_us(dj_set_end, duoji_now * 500);
		}
		else if (duoji_now < duoji_need)
		{
			duoji_now = duoji_now + 1;
			digitalWrite(duoji, HIGH);
			set_timer1_us(dj_set_end, duoji_now * 500);
		}
	}
}