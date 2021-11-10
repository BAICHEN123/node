#include "usermain.h"
extern "C"
{
	const char *MODE_INFO = "@断电记忆[0-2]:关闭，仅本次，所有@水位[0-1]:正常，过低@水泵模式[0-1]:手动，定时，禁用";
	struct Udpwarn shuiwei_low_error = {WARN, NOT_WARN, 0, 1, "水位过低，请及时补水"};

	uint8_t power_save = 0; //断电记忆

	//两个开关，当他为2时，是自动模式，其他时候读取12 和14号脚的电平
	double guang_qiang = 0; //光照强度
	uint8_t shui_wei = 0;
	uint8_t shui_beng = 0;
	uint8_t shui_beng_mode = 0;
	uint8_t s_b_time1 = 0;
	uint8_t yu_men[4 * 2 + 3 * 2] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //与门寄存器

	//下面定义几个引脚的功能
	const uint8_t p_shuiwei = 14; //水位
	const uint8_t jd2 = 12;		  //继电器
	const uint8_t anjian1 = 0;	  //按键1输入

	//最大值最小值务必和数据项目的数据类型保持一致
	unsigned char CONST1[4] = {0, 1, 2, 100};

	struct MyType data_list[MAX_NAME] = {
		{"光强", "%", TYPE_DOUBLE, sizeof(guang_qiang), &guang_qiang, NULL, NULL},
		{"水位", NULL, TYPE_u8, sizeof(shui_wei), &shui_wei, CONST1, CONST1 + 1},
		{"@水泵模式", NULL, TYPE_u8, sizeof(shui_beng_mode), &shui_beng_mode, CONST1, CONST1 + 2},
		{"@水泵", NULL, TYPE_u8, sizeof(shui_beng), &shui_beng, CONST1, CONST1 + 1},
		{"@水泵自动关闭计时", "S", TYPE_u8, sizeof(s_b_time1), &s_b_time1, CONST1, CONST1 + 3},
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
		static uint8 shui_wei_count = 0;
		clear_wifi_data(wifi_ssid_pw_file); //长按按键1清除wifi账号密码记录
		shui_wei = digitalRead(p_shuiwei);
		guang_qiang = system_adc_read() * 100.00 / 1024;

		if (shui_wei == 1)
		{
			shui_wei_count = shui_wei_count + 1;
			if (shui_wei_count > 25) //连续多次检测到低水位
			{
				if (shuiwei_low_error.status == NOT_WARN)
				{
					set_warn(&shuiwei_low_error);
				}
				shui_wei_count = 25;
			}
		}
		else
		{
			shui_wei_count = 0;
			shuiwei_low_error.status = NOT_WARN;
		}
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
		add_value(&shui_beng, sizeof(shui_beng));
		add_value(&shui_beng_mode, sizeof(shui_beng_mode));
	}

	//每隔 1s
	void ruan_timer_1s()
	{
		if (shui_beng_mode == 1)
		{
			if (s_b_time1 > 0)
			{
				shui_beng = 1;
				digitalWrite(jd2, LOW);
				s_b_time1 = s_b_time1 - 1;
			}
			else
			{
				shui_beng = 1;
				digitalWrite(jd2, HIGH);
			}
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
		// digitalWrite(jd1, LED1);
		// digitalWrite(jd2, LED2);

		yu_men[3] = yu_men[0] & yu_men[1] & yu_men[2];
		yu_men[7] = yu_men[4] & yu_men[5] & yu_men[6];
		yu_men[10] = yu_men[8] & yu_men[9];
		yu_men[13] = yu_men[11] & yu_men[12];

		//不要在这里采集数据，模拟信号有很大的误差
		//shui_wei = digitalRead(p_shuiwei);
		//guang_qiang = system_adc_read() * 100 / 1024.00;

		if (shui_beng_mode == 0)
		{ //手动模式
			if (shui_beng == 1)
			{
				digitalWrite(jd2, LOW);
			}
			else
			{
				digitalWrite(jd2, HIGH);
			}
			s_b_time1 = 0;
		}
		else if (shui_beng_mode == 1)
		{ //定时模式
			if (s_b_time1 > 0)
			{
				shui_beng = 1;
				digitalWrite(jd2, LOW);
			}
			else
			{
				shui_beng = 0;
				digitalWrite(jd2, HIGH);
			}
		}
		else
		{ //禁用
			shui_beng = 0;
			digitalWrite(jd2, HIGH);
		}
	}
}
