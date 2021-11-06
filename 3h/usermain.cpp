#include "usermain.h"
extern "C"
{
	const char *MODE_INFO = "@断电记忆[0-2]:关闭，仅本次，所有@水位[0-1]:正常，过低";

	uint8_t power_save = 0; //断电记忆

	//两个开关，当他为2时，是自动模式，其他时候读取12 和14号脚的电平
	double guang_qiang = 0; //光照强度
	uint8_t shui_wei = 0;
	uint8_t LED2 = 0;
	uint8_t yu_men[4 * 2 + 3 * 2] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //与门寄存器

	//下面定义几个引脚的功能
	const uint8_t huo_er = 14; //1号继电器
	const uint8_t jd2 = 12;	   //2号继电器
	const uint8_t anjian1 = 0; //按键1输入

	unsigned char CONST1[3] = {0, 1, 2};

	struct MyType data_list[MAX_NAME] = {
		{"光强", "%", TYPE_DOUBLE, sizeof(guang_qiang), &guang_qiang, NULL, NULL},
		{"水位", NULL, TYPE_u8, sizeof(shui_wei), &shui_wei, CONST1, CONST1 + 1},
		{"@水泵", NULL, TYPE_u8, sizeof(LED2), &LED2, CONST1, CONST1 + 1},
		{"@断电记忆", NULL, TYPE_u8, sizeof(power_save), &power_save, CONST1, CONST1 + 2},
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
		shui_wei = digitalRead(huo_er);
		guang_qiang = system_adc_read() * 100.00 / 1024;
	}

	void my_init()
	{
		//pinMode(yan_wu, INPUT);
		pinMode(anjian1, INPUT); //按键1
		//pinMode(hongwai_renti, INPUT);
		pinMode(jd2, OUTPUT);
		//pinMode(jd1, OUTPUT);
		set_timer1_ms(timer1_worker, TIMER1_timeout_ms); //强制重新初始化定时中断，如果单纯的使用 dht11_get 里的过程初始化，有概率初始化失败
	}

	void add_values()
	{
		//add_value(&LED1, sizeof(LED1));
		add_value(&LED2, sizeof(LED2));
	}

	//每隔 1s
	void ruan_timer_1s()
	{
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
		// digitalWrite(jd1, LED1);
		// digitalWrite(jd2, LED2);

		yu_men[3] = yu_men[0] & yu_men[1] & yu_men[2];
		yu_men[7] = yu_men[4] & yu_men[5] & yu_men[6];
		yu_men[10] = yu_men[8] & yu_men[9];
		yu_men[13] = yu_men[11] & yu_men[12];

		shui_wei = digitalRead(huo_er);

		guang_qiang = system_adc_read() * 100 / 1024.00;

		if (LED2 == 1)
		{
			digitalWrite(jd2, LOW);
		}
		else
		{
			digitalWrite(jd2, HIGH);
		}
	}
}
