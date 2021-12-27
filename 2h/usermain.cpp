#include "usermain.h"
extern "C"
{
	/*
const char *str_data_names[MAX_NAME] = {
	"红外人体检测",
	"烟雾逻辑",
	"烟雾模拟",
	"@开关1[0-1]",
	"@开关2[0-1]",
	"@断电记忆[0-2]"
	};
	*/
	const char *MODE_INFO = "@断电记忆[0-2]:关闭，仅本次，所有@红外人体检测[0-1]:无，有@烟雾逻辑[0-1]:有，无";

	uint8_t power_save = 0; //断电记忆

	//两个开关，当他为2时，是自动模式，其他时候读取12 和14号脚的电平
	uint8_t LED1 = 0;
	uint8_t LED2 = 0;
	double yanwu_my = 0;
	uint8_t yanwu = 0;
	uint8_t hongwai = 0;

	//下面定义几个引脚的功能
	const uint8_t jd1 = 14;			 //1号继电器
	const uint8_t jd2 = 12;			 //2号继电器
	const uint8_t yan_wu = 13;		 //烟雾逻辑输入
	const uint8_t hongwai_renti = 5; //红外人体
	const uint8_t anjian1 = 0;		 //按键1输入

	unsigned char CONST1[3]={0,1,2};

	struct MyType data_list[MAX_NAME] = {
		{"红外人体检测", NULL, TYPE_u8, sizeof(hongwai), &hongwai,CONST1,CONST1+1},
		{"烟雾逻辑", NULL, TYPE_u8, sizeof(yanwu), &yanwu,CONST1,CONST1+1},
		{"烟雾模拟", "%", TYPE_DOUBLE, sizeof(yanwu_my), &yanwu_my,NULL,NULL},
		{"@开关1", NULL, TYPE_u8, sizeof(LED1), &LED1,CONST1,CONST1+1},
		{"@水泵", NULL, TYPE_u8, sizeof(LED2), &LED2,CONST1,CONST1+1},
		{"@断电记忆", NULL, TYPE_u8, sizeof(power_save), &power_save,CONST1,CONST1+2}

	};

	void timer1_worker()
	{
		clear_wifi_data(wifi_ssid_pw_file); //长按按键1清除wifi账号密码记录
		yanwu_my = system_adc_read() * 100 / 1024.00;
		yanwu = digitalRead(yan_wu);
		hongwai = digitalRead(hongwai_renti);
	}

	void my_init()
	{
		pinMode(yan_wu, INPUT);
		pinMode(anjian1, INPUT); //按键1
		pinMode(hongwai_renti, INPUT);
		pinMode(jd2, OUTPUT);
		pinMode(jd1, OUTPUT);
		set_timer1_ms(timer1_worker, TIMER1_timeout_ms); //强制重新初始化定时中断，如果单纯的使用 dht11_get 里的过程初始化，有概率初始化失败
	}

	void add_values()
	{
		add_value(&LED1, sizeof(LED1));
		add_value(&LED2, sizeof(LED2));
	}

	//每隔 RUAN_TIMEer_ms
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
		if(LED1==1)
		{
			digitalWrite(jd1, LOW);
		}
		else
		{
			digitalWrite(jd1, HIGH);

		}

		if(LED2==1)
		{
			digitalWrite(jd2, LOW);
		}
		else
		{
			digitalWrite(jd2, HIGH);

		}
	}
}
