#include "usermain.h"
extern "C"
{
	const char *MODE_INFO = "@1号土壤湿度[0-1]:湿润，干燥@2号土壤湿度[0-1]:湿润，干燥@3号土壤湿度[0-1]:湿润，干燥";

	uint8_t power_save = 0; //断电记忆

	//两个开关，当他为2时，是自动模式，其他时候读取12 和14号脚的电平
	uint8_t shi_du1 = 0;
	uint8_t shi_du2 = 0;
	uint8_t shi_du3 = 0;
	double shi_du_a = 0;

	//下面定义几个引脚的功能
	const uint8_t shudu1 = 13;			 //1号继电器
	const uint8_t shudu2 = 12;			 //1号继电器
	const uint8_t shudu3 = 14;			 //1号继电器
	const uint8_t anjian1 = 0;		 //按键1输入

	unsigned char CONST1[2]={0,1};

	struct MyType data_list[MAX_NAME] = {
		//{"红外人体检测", NULL, TYPE_u8, sizeof(hongwai), &hongwai,CONST1,CONST1+1},
		//{"烟雾逻辑", NULL, TYPE_u8, sizeof(yanwu), &yanwu,CONST1,CONST1+1},
		//{"烟雾模拟", "%", TYPE_DOUBLE, sizeof(yanwu_my), &yanwu_my,NULL,NULL},
		//{"@开关1", NULL, TYPE_u8, sizeof(LED1), &LED1,CONST1,CONST1+1},
		{"1号土壤湿度", NULL, TYPE_u8, sizeof(shi_du1), &shi_du1,CONST1,CONST1+1},
		{"2号土壤湿度", NULL, TYPE_u8, sizeof(shi_du2), &shi_du2,CONST1,CONST1+1},
		{"3号土壤湿度", NULL, TYPE_u8, sizeof(shi_du3), &shi_du3,CONST1,CONST1+1},
		{"1号详细土壤湿度", "%", TYPE_DOUBLE, sizeof(shi_du_a), &shi_du_a,NULL,NULL}
		//{"@断电记忆", NULL, TYPE_u8, sizeof(power_save), &power_save,CONST1,CONST1+2}

	};

	void timer1_worker()
	{
		clear_wifi_data(wifi_ssid_pw_file); //长按按键1清除wifi账号密码记录
		shi_du1=digitalRead(shudu1);
		shi_du2=digitalRead(shudu2);
		shi_du3=digitalRead(shudu3);
		shi_du_a=system_adc_read() * 100 / 1024.00;
		//yanwu_my = system_adc_read() * 100 / 1024.00;
	}

	void my_init()
	{
		//pinMode(yan_wu, INPUT);
		pinMode(anjian1, INPUT); //按键1
		pinMode(shudu1, INPUT); //按键1
		pinMode(shudu2, INPUT); //按键1
		pinMode(shudu3, INPUT); //按键1
		//pinMode(hongwai_renti, INPUT);
		set_timer1_ms(timer1_worker, TIMER1_timeout_ms); //强制重新初始化定时中断，如果单纯的使用 dht11_get 里的过程初始化，有概率初始化失败
	}

	void add_values()
	{
		//add_value(&LED1, sizeof(LED1));
		//add_value(&LED2, sizeof(LED2));
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
