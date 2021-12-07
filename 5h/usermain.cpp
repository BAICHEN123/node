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

	uint8_t ji_shi1 = 0; //倒计时
	uint8_t ji_shi2 = 0; //倒计时

	uint8_t yu_men[4 * 2 + 3 * 2] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //与门寄存器

	//定义几个引脚的功能
	const uint8_t tongfeng = 14; //继电器
	const uint8_t duoji = 5;	 //舵机
	const uint8_t anjian1 = 0;	 //按键1输入

	unsigned char CONST1[5] = {0, 1, 2, 5, 120};

	struct MyType data_list[MAX_NAME] = {
		{"光强", "%", TYPE_DOUBLE, sizeof(guang_qiang), &guang_qiang, NULL, NULL},
		{"@五叶窗", NULL, TYPE_u8, sizeof(duoji_need), &duoji_need, CONST1 + 1, CONST1 + 3},
		{"@通风扇", NULL, TYPE_u8, sizeof(tong_feng), &tong_feng, CONST1, CONST1 + 1},
		{"@断电记忆", NULL, TYPE_u8, sizeof(power_save), &power_save, CONST1, CONST1 + 2},

		{"@1号倒计时", NULL, TYPE_u8, sizeof(ji_shi1), &ji_shi1, CONST1, CONST1 + 4}, //1号倒计时
		{"@2号倒计时", NULL, TYPE_u8, sizeof(ji_shi2), &ji_shi2, CONST1, CONST1 + 4}, //2号倒计时

		{"@1与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men, CONST1, CONST1 + 1},		//1号与门1号入口
		{"@1与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 1, CONST1, CONST1 + 1},	//1号与门2号入口
		{"@1与3入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 2, CONST1, CONST1 + 1},	//1号与门3号入口
		{"1与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 3, CONST1, CONST1 + 1},	//1号与门输出
		{"@2与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 4, CONST1, CONST1 + 1},	//2号与门1号入口
		{"@2与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 5, CONST1, CONST1 + 1},	//2号与门2号入口
		{"@2与3入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 6, CONST1, CONST1 + 1},	//2号与门3号入口
		{"2与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 7, CONST1, CONST1 + 1},	//2号与门输出
		{"@3与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 8, CONST1, CONST1 + 1},	//3号与门1号入口
		{"@3与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 9, CONST1, CONST1 + 1},	//3号与门2号入口
		{"3与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 10, CONST1, CONST1 + 1},	//3号与门输出
		{"@4与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 11, CONST1, CONST1 + 1}, //4号与门1号入口
		{"@4与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 12, CONST1, CONST1 + 1}, //4号与门2号入口
		{"4与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 13, CONST1, CONST1 + 1},	//4号与门输出
		{NULL}																			//到这里结束

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
		add_value(&duoji_need, sizeof(duoji_need));
		//记录通风扇的状态
		add_value(&tong_feng, sizeof(tong_feng));
	}

	void ruan_timer_1s()
	{
		if (ji_shi1 > 0)
		{
			ji_shi1 = ji_shi1 - 1;
		}
		if (ji_shi2 > 0)
		{
			ji_shi2 = ji_shi2 - 1;
		}
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

		yu_men[3] = yu_men[0] & yu_men[1] & yu_men[2];
		yu_men[7] = yu_men[4] & yu_men[5] & yu_men[6];
		yu_men[10] = yu_men[8] & yu_men[9];
		yu_men[13] = yu_men[11] & yu_men[12];

		if (tong_feng == 1)
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
