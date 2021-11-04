#include "usermain.h"
extern "C"
{
	const char *MODE_INFO = "@开关模式[0-3]:手动，声控，光控，光声混控@断电记忆[0-2]:关闭，仅本次，所有";
	uint8_t power_save = 0; //断电记忆

	struct DHT11_data dht11_data = {666, 666};
	double liangdu = 0;
	//两个开关，当他为2时，是自动模式，其他时候读取12 和14号脚的电平
	//uint8_t LED1 = 0;
	//uint8_t switch_1 = 2;
	uint8_t LED2 = 0;
	uint8_t switch_2 = 2;
	short switch_light_up_TIME_s = 30;	//重新加载的值//声控灯开启时长
	short switch_light_up_time_x_s = 0; //计数器用
	short TEMPERATURE_ERROR_HIGH = 40;
	short TEMPERATURE_ERROR_LOW = 10;
	uint8_t light_io = 0; //补光区间
	uint8_t test = 0;
	uint8_t duoji_need = 5; //舵机的期望值
	uint8_t duoji_now = 0;	//舵机的期望值
	static int beeeeee = 0;

	uint8_t yu_men[3 * 2] = {0, 0, 0, 0, 0, 0}; //与门寄存器

	//下面定义几个引脚的功能
	const uint8_t jd1 = 14;		//1号继电器
	const uint8_t jd2 = 12;		//2号继电器
	const uint8_t light = 13;	//光敏逻辑输入
	const uint8_t shengyin = 4; //声音逻辑输入
	const uint8_t anjian1 = 0;	//按键1输入
	const uint8_t dht11 = 5;	//dht11
	const uint8_t duoji = 15;	//舵机

	//关于这里和后面的使用时的警告，将其定义为const完全不影响使用，但是会出现警告，只要自己不要在后面的使用过程中对其赋值就没有问题
	uint8_t CONST1[6] = {0, 1, 2, 3};
	short CONST2[3] = {0, 45, 300};
	double CONST3[2] = {10, 100};

	struct MyType data_list[MAX_NAME] = {
		{"时", "h", TYPE_u8, sizeof(Now.hour), &(Now.hour), NULL, NULL},
		{"分", "m", TYPE_u8, sizeof(Now.minute), &(Now.minute), NULL, NULL},
		{"秒", "s", TYPE_u8, sizeof(Now.sec), &(Now.sec), NULL, NULL},
		{"温度", "°C", TYPE_FLOAT, sizeof(dht11_data.temperature), &(dht11_data.temperature), NULL, NULL},
		{"湿度", "%", TYPE_FLOAT, sizeof(dht11_data.humidity), &(dht11_data.humidity), NULL, NULL},
		{"烟雾浓度", "%", TYPE_DOUBLE, sizeof(liangdu), &liangdu, NULL, NULL},
		{"@开关", NULL, TYPE_u8, sizeof(LED2), &LED2, CONST1, CONST1 + 1},
		{"@开关模式", NULL, TYPE_u8, sizeof(switch_2), &switch_2, CONST1, CONST1 + 3},
		{"@亮度输入", NULL, TYPE_u8, sizeof(light_io), &light_io, CONST1, CONST1 + 1},
		{"声控灯剩余时长/S", "S", TYPE_SHORT, sizeof(switch_light_up_time_x_s), &switch_light_up_time_x_s, NULL, NULL},
		{"@声控灯时长/S", "S", TYPE_SHORT, sizeof(switch_light_up_TIME_s), &switch_light_up_TIME_s, CONST2, CONST2 + 2},
		{"@高温警告/°C", "°C", TYPE_SHORT, sizeof(TEMPERATURE_ERROR_HIGH), &TEMPERATURE_ERROR_HIGH, &TEMPERATURE_ERROR_LOW, CONST2 + 1},
		{"@低温警告/°C", "°C", TYPE_SHORT, sizeof(TEMPERATURE_ERROR_LOW), &TEMPERATURE_ERROR_LOW, CONST2, &TEMPERATURE_ERROR_HIGH},
		{"@断电记忆", NULL, TYPE_u8, sizeof(power_save), &power_save, CONST1, CONST1 + 2},
		{"@test1", NULL, TYPE_u8, sizeof(test), &test, CONST1, CONST1 + 1},			   //测试错误并发用
		{"@1与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men, CONST1, CONST1 + 1},	   //1号与门1号入口
		{"@1与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 1, CONST1, CONST1 + 1}, //1号与门2号入口
		{"1与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 2, CONST1, CONST1 + 1},   //1号与门输出
		{"@2与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 3, CONST1, CONST1 + 1}, //2号与门1号入口
		{"@2与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 4, CONST1, CONST1 + 1}, //2号与门2号入口
		{"2与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 5, CONST1, CONST1 + 1},   //2号与门输出
		{NULL}																		   //到这里结束

	};

	//更新与门的状态
	void yu_men_re()
	{
		yu_men[2] = yu_men[0] & yu_men[1];
		yu_men[5] = yu_men[3] & yu_men[4];
	}

	void my_init()
	{

		refresh_work(); //初始化引脚之前，先调整高低电平，减少不必要的继电器响声

		dht11_init(dht11);		  //这个是DHT11.h/DHT11.c里的函数，初始化引脚
		pinMode(light, INPUT);	  //光
		pinMode(anjian1, INPUT);  //按键1
		pinMode(shengyin, INPUT); //d2 声音
		pinMode(jd2, OUTPUT);
		//pinMode(jd1, OUTPUT);
		set_timer1_ms(timer1_worker, TIMER1_timeout_ms); //强制重新初始化定时中断，如果单纯的使用 dht11_get 里的过程初始化，有概率初始化失败
		//（仅在程序复位的时候可以成功，原因：timer2_count 没有复位就不会被初始化，自然调用不到定时器的初始化函数），
		dht11_get(); //读取dht11的数据，顺便启动定时器//这里有问题，当断网重连之后，定时器函数有可能不会被重新填充
	}

	void ruan_timer_1s()
	{
	}

	void ruan_timer_ms()
	{
		if (beeeeee > 10) //要大于五是因为偶尔会采样出错，一般是连续的三个，正常人发出的声音远大于此
		{
			//更新声控灯剩余时长
			switch_light_up_time_x_s = switch_light_up_TIME_s;
		}
		beeeeee = 0;

		if (test == 1)
		{
			static struct Udpwarn test111 = {WARN, NOT_WARN, 0, 3, "test111111"};
			static struct Udpwarn test222 = {WARN, NOT_WARN, 0, 4, "test2222222"};
			if (test111.status == WARN_ACK)
			{
				test111.status = NOT_WARN;
			}
			else
			{
				test111.status = IS_WARN;
				set_warn(&test111);
			}

			if (test222.status == WARN_ACK)
			{
				test222.status = NOT_WARN;
			}
			else
			{
				test222.status = IS_WARN;
				set_warn(&test222);
			}
			test = 0;
		}
	}

	void ruan_timer_us()
	{
		beeeeee = beeeeee + digitalRead(shengyin);
	}

	/*
	此函数根据光强返回是否需要开灯
	1	开灯
	0	关灯
	*/
	uint8_t light_high()
	{
		return light_io;
	}

	/*
	此函数根据声音返回是否需要开灯
	1	开灯
	0	关灯
	*/
	uint8_t sheng_yin_high()
	{
		if (switch_light_up_time_x_s > 0)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	/*
	此函数根据 声音 和光强  返回是否需要开灯
	1	开灯
	0	关灯
	*/
	uint8_t shengyin_and_light_high()
	{
		return sheng_yin_high() && light_high();
	}

	//声控灯倒计时 计数器 倒数
	void shengyin_timeout_out()
	{
		static const int COUNT = 1000 / TIMER1_timeout_ms;
		static int count = COUNT;
		if (switch_light_up_time_x_s > 0)
		{
			if (count == 0)
			{
				count = COUNT;
				switch_light_up_time_x_s--;
			}
			count--;
		}
	}

	//根据 传感器状态 和 用户选择模式 控制继电器
	void set_jdq(const uint8_t pin_x, short switch_x, uint8_t *LEDx)
	{
		switch (switch_x)
		{
		case 1:
			*LEDx = sheng_yin_high();
			break;
		case 2:
			*LEDx = light_high();
			break;
		case 3:
			*LEDx = shengyin_and_light_high();
			break;
		}

		if (*LEDx == 1)
		{
			digitalWrite(pin_x, LOW); //继电器低电平的时候导通
		}
		else
		{
			digitalWrite(pin_x, HIGH);
		}
	}

	/*根据模式更新继电器开关的状态*/
	void refresh_work()
	{
		//set_jdq(jd1, switch_1, &LED1);
		//digitalWrite(jd1, !LED1); //默认高电平，不初始化的时候也是高电平，进行取反操作，这样的话手机app上就会显示正常。
		set_jdq(jd2, switch_2, &LED2);
		yu_men_re();
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
			Serial.print("DHT11 error :timeout 超时未回复\n");
			//UDP_Send(MYHOST, UDP_PORT, "error:DHT11 timeout_back");//---------------------这里还要统一通讯协议，当设备的驱动报错的时候需要的信号是什么样子的，当传感器的数据异常的的时候的信号是什么样子的
		}
		else if (t == -1)
		{
			Serial.print("DHT11 error :sum error 数据校验错误\n");
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

	/*	定时器工作内容	*/
	void timer1_worker()
	{
		//delay(20);//时间中断函数里不可以用delay
		clear_wifi_data(wifi_ssid_pw_file); //长按按键1清除wifi账号密码记录
		refresh_work();						//更新继电器状态
		shengyin_timeout_out();				//更新声控灯倒计时
		//调用DHT11的读取函数
		if (dht11_get())
		{
			//dj_set();
		}
		liangdu = system_adc_read() * 100 / 1024.00;
	}

	/*	此函数在定时中断中调用，处理温湿度传感器通讯协议中18ms下拉	*/
	void read_dht11()
	{
		//让DHT11的信号引脚拉低，等待20ms，之后调用get_DHT11_DATA() 开始正式调用读取函数
		set_timer1_ms(timer1_worker, TIMER1_timeout_ms - LOW_PIN_ms); //正常时间之后恢复 timer1_worker 的工作
		DHT11_read_and_send();
	}

	/*添加需要保存到flash的变量，上限为 list_values_len_max */
	void add_values()
	{
		add_value(&switch_2, sizeof(switch_2));
		//add_value(&LED1, sizeof(LED1));
		add_value(&LED2, sizeof(LED2));
		add_value(&switch_light_up_TIME_s, sizeof(switch_light_up_TIME_s));
		add_value(&TEMPERATURE_ERROR_HIGH, sizeof(TEMPERATURE_ERROR_HIGH));
		add_value(&TEMPERATURE_ERROR_LOW, sizeof(TEMPERATURE_ERROR_LOW));
		add_value(&light_io, sizeof(light_io));
		add_value(yu_men, sizeof(yu_men));
	}
}