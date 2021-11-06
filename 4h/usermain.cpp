#include "usermain.h"
extern "C"
{
	const char *MODE_INFO = "@1号土壤湿度[0-1]:湿润，干燥@2号土壤湿度[0-1]:湿润，干燥@3号土壤湿度[0-1]:湿润，干燥";

	uint8_t power_save = 0; //断电记忆

	//定义传感器储存变量
	uint8_t shi_du1 = 0;
	uint8_t shi_du2 = 0;
	uint8_t shi_du3 = 0;
	double shi_du_a = 0;
	struct DHT11_data dht11_data = {666, 666};

	short TEMPERATURE_ERROR_HIGH = 40;
	short TEMPERATURE_ERROR_LOW = 10;

	uint8_t yu_men[4 * 2 + 3] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //与门寄存器

	//下面定义几个引脚的功能
	const uint8_t shudu1 = 13; //1号继电器
	const uint8_t shudu2 = 12; //1号继电器
	const uint8_t shudu3 = 14; //1号继电器
	const uint8_t dht11 = 5;   //DHT11输入
	const uint8_t anjian1 = 0; //按键1输入

	unsigned char CONST1[2] = {0, 1};
	short CONST2[2] = {0, 45};

	struct MyType data_list[MAX_NAME] = {
		{"1号详细土壤湿度", "%", TYPE_DOUBLE, sizeof(shi_du_a), &shi_du_a, NULL, NULL},
		{"1号土壤湿度", NULL, TYPE_u8, sizeof(shi_du1), &shi_du1, CONST1, CONST1 + 1},
		{"2号土壤湿度", NULL, TYPE_u8, sizeof(shi_du2), &shi_du2, CONST1, CONST1 + 1},
		{"3号土壤湿度", NULL, TYPE_u8, sizeof(shi_du3), &shi_du3, CONST1, CONST1 + 1},
		{"温度", "°C", TYPE_FLOAT, sizeof(dht11_data.temperature), &(dht11_data.temperature), NULL, NULL},
		{"湿度", "%", TYPE_FLOAT, sizeof(dht11_data.humidity), &(dht11_data.humidity), NULL, NULL},
		{"@高温警告/°C", "°C", TYPE_SHORT, sizeof(TEMPERATURE_ERROR_HIGH), &TEMPERATURE_ERROR_HIGH, &TEMPERATURE_ERROR_LOW, CONST2 + 1},
		{"@低温警告/°C", "°C", TYPE_SHORT, sizeof(TEMPERATURE_ERROR_LOW), &TEMPERATURE_ERROR_LOW, CONST2, &TEMPERATURE_ERROR_HIGH},
		{"@断电记忆", NULL, TYPE_u8, sizeof(power_save), &power_save, CONST1, CONST1 + 2},
		{"@1与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men, CONST1, CONST1 + 1},	   //1号与门1号入口
		{"@1与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 1, CONST1, CONST1 + 1}, //1号与门2号入口
		{"@1与3入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 2, CONST1, CONST1 + 1}, //1号与门3号入口
		{"1与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 3, CONST1, CONST1 + 1},   //1号与门输出
		{"@2与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 4, CONST1, CONST1 + 1}, //2号与门1号入口
		{"@2与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 5, CONST1, CONST1 + 1}, //2号与门2号入口
		{"@2与3入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 6, CONST1, CONST1 + 1}, //2号与门3号入口
		{"2与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 7, CONST1, CONST1 + 1},   //2号与门输出
		{"@3与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 8, CONST1, CONST1 + 1}, //3号与门1号入口
		{"@3与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 9, CONST1, CONST1 + 1}, //3号与门2号入口
		{"3与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 10, CONST1, CONST1 + 1},  //3号与门输出
		{"时", NULL, TYPE_u8, sizeof(Now.hour), &(Now.hour), NULL, NULL},
		{"分", NULL, TYPE_u8, sizeof(Now.minute), &(Now.minute), NULL, NULL},
		{"秒", NULL, TYPE_u8, sizeof(Now.sec), &(Now.sec), NULL, NULL},
		{"星期", NULL, TYPE_u8, sizeof(Now.week), &(Now.week), NULL, NULL},
		{"日", NULL, TYPE_u8, sizeof(Now.day), &(Now.day), NULL, NULL},
		{"月", NULL, TYPE_u8, sizeof(Now.month), &(Now.month), NULL, NULL},
		{"年", NULL, TYPE_USHORT, sizeof(Now.year), &(Now.year), NULL, NULL},
		{NULL} //到这里结束

	};

	void timer1_worker()
	{
		clear_wifi_data(wifi_ssid_pw_file); //长按按键1清除wifi账号密码记录
		shi_du1 = digitalRead(shudu1);
		shi_du2 = digitalRead(shudu2);
		shi_du3 = digitalRead(shudu3);
		shi_du_a = system_adc_read() * 100 / 1024.00;

		if (dht11_get())
		{
			//可以在这里调用其他定时延后执行的任务
		}
		//yanwu_my = system_adc_read() * 100 / 1024.00;
	}

	void my_init()
	{
		//pinMode(yan_wu, INPUT);
		pinMode(anjian1, INPUT); //按键1
		pinMode(shudu1, INPUT);	 //按键1
		pinMode(shudu2, INPUT);	 //按键1
		pinMode(shudu3, INPUT);	 //按键1
		dht11_init(dht11);		 //这个是DHT11.h/DHT11.c里的函数，初始化引脚
		//pinMode(hongwai_renti, INPUT);
		set_timer1_ms(timer1_worker, TIMER1_timeout_ms); //强制重新初始化定时中断，如果单纯的使用 dht11_get 里的过程初始化，有概率初始化失败
	}

	void add_values()
	{
		add_value(&TEMPERATURE_ERROR_LOW, sizeof(TEMPERATURE_ERROR_LOW));
		add_value(&TEMPERATURE_ERROR_HIGH, sizeof(TEMPERATURE_ERROR_HIGH));
	}

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
		yu_men[3] = yu_men[0] & yu_men[1] & yu_men[2];
		yu_men[7] = yu_men[4] & yu_men[5] & yu_men[6];
		yu_men[10] = yu_men[8] & yu_men[9];
	}

	/*此函数在定时中断中调用，处理温湿度传感器的40bit读取*/
	void DHT11_read_and_send()
	{
		//读取温湿度，并将异常情况返回
		short t = dht11_read_data(&dht11_data);
		//+EID=20,chip_id=2507829w2,temperature high
		static struct Udpwarn temperature_low_error = {WARN, NOT_WARN, 0, 1, "温度低于设定值"};
		static struct Udpwarn temperature_high_error = {WARN, NOT_WARN, 0, 2, "温度高于设定值"};
		if (t == 0)
		{
			Serial.print("DHT11 error :timeout 超时未回复\r\n");
			//UDP_Send(MYHOST, UDP_PORT, "error:DHT11 timeout_back");//---------------------这里还要统一通讯协议，当设备的驱动报错的时候需要的信号是什么样子的，当传感器的数据异常的的时候的信号是什么样子的
		}
		else if (t == -1)
		{
			Serial.print("DHT11 error :sum error 数据校验错误\r\n");
			//UDP_Send(MYHOST, UDP_PORT, "error:DHT11 sum_erroe");
		}
		else if (EID > 0)
		{
			if (dht11_data.temperature > TEMPERATURE_ERROR_HIGH)
			{
				if (temperature_high_error.status == NOT_WARN)
				{
					//temperature_high_error.status = IS_WARN;
					set_warn(&temperature_high_error);
				}
			}
			else if (dht11_data.temperature < TEMPERATURE_ERROR_LOW)
			{
				if (temperature_low_error.status == NOT_WARN)
				{
					//temperature_low_error.status = IS_WARN;
					set_warn(&temperature_low_error);
				}
			}
			else
			{
				//可以在这里插入警报恢复的提示
				temperature_low_error.status = NOT_WARN;
				temperature_high_error.status = NOT_WARN;
			}
		}
	}

	/*每隔 DHT11_SPACE_OF_TIME_ms 读取DHT11*/
	int dht11_get()
	{
		static short timer2_count = TIMER2_COUNT; //
		timer2_count++;
		if (timer2_count >= TIMER2_COUNT)
		{
			//先拉低，LOW_PIN_ms 之后调用读取函数
			set_timer1_ms(read_dht11, (unsigned int)dht11_read_ready());
			timer2_count = 0;
			return 0;
		}
		return 1;
	}

	/*	此函数在定时中断中调用，处理温湿度传感器通讯协议中18ms下拉	*/
	void read_dht11()
	{
		//让DHT11的信号引脚拉低，等待20ms，之后调用get_DHT11_DATA() 开始正式调用读取函数
		set_timer1_ms(timer1_worker, TIMER1_timeout_ms - LOW_PIN_ms); //正常时间之后恢复 timer1_worker 的工作
		DHT11_read_and_send();
	}
}
