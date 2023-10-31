#include "usermain.h"
const char *MODE_INFO = "@断电记忆[0-2]:关闭，仅本次，所有";
static struct Udpwarn user_error1 = {WARN, NOT_WARN, 0, 5, "1 号自定义警告被触发"};
static struct Udpwarn user_error2 = {WARN, NOT_WARN, 0, 6, "2 号自定义警告被触发"};

// 定义传感器储存变量
uint8_t power_save = 0; // 断电记忆
uint8_t user_error_1 = 0;
uint8_t user_error_2 = 0;
struct DHT11_data dht11_data = {666, 666};
double yan_wu_A = 0;
double yanwu_my = 0;

// 定义几个引脚的功能
const uint8_t anjian1 = 0; // 按键1输入
const uint8_t dht11 = 5;   // dht11

// 定义几个范围定义变量
unsigned char CONST1[3] = {0, 1, 2};
int dht11_get();
void read_dht11();

struct MyType data_list[MAX_NAME] = {
	{"温度", "°C", TYPE_FLOAT, sizeof(dht11_data.temperature), &(dht11_data.temperature), NULL, NULL},
	{"湿度", "%", TYPE_FLOAT, sizeof(dht11_data.humidity), &(dht11_data.humidity), NULL, NULL},
	{"烟雾模拟", "%", TYPE_DOUBLE, sizeof(yanwu_my), &yanwu_my, NULL, NULL},
	{"@1号自定义警告", NULL, TYPE_u8, sizeof(user_error_1), &user_error_1, CONST1, CONST1 + 1}, // 用户自定义警告
	{"@2号自定义警告", NULL, TYPE_u8, sizeof(user_error_2), &user_error_2, CONST1, CONST1 + 1}, // 用户自定义警告
	{"时", NULL, TYPE_u8, sizeof(Now.hour), &(Now.hour), NULL, NULL},
	{"分", NULL, TYPE_u8, sizeof(Now.minute), &(Now.minute), NULL, NULL},
	{"秒", NULL, TYPE_u8, sizeof(Now.sec), &(Now.sec), NULL, NULL},
	{"@断电记忆", NULL, TYPE_u8, sizeof(power_save), &power_save, CONST1, CONST1 + 2},
	{NULL} // 到这里结束

};

void timer1_worker()
{
	// 长按按键1清除wifi账号密码记录
	clear_wifi_data(wifi_ssid_pw_file);

	yanwu_my = system_adc_read() * 100 / 1024.00;

	if (dht11_get())
	{
		// 可以在这里调用其他定时延后执行的任务
	}
}

/*请注意引脚初始化和赋值的先后顺序，不然可能导致外接的继电器闪烁
 */
void my_init()
{
	refresh_work(); // 初始化引脚之前，先调整高低电平，减少不必要的继电器响声

	dht11_init(dht11);		 // 这个是DHT11.h/DHT11.c里的函数，初始化引脚
	pinMode(anjian1, INPUT); // 按键1
	// pinMode(jd1, OUTPUT);
	set_timer1_ms(timer1_worker, TIMER1_timeout_ms); // 强制重新初始化定时中断，如果单纯的使用 dht11_get 里的过程初始化，有概率初始化失败
	// （仅在程序复位的时候可以成功，原因：timer2_count 没有复位就不会被初始化，自然调用不到定时器的初始化函数），
	dht11_get(); // 读取dht11的数据，顺便启动定时器//这里有问题，当断网重连之后，定时器函数有可能不会被重新填充
}

// 需要断电记忆的变量在这里添加
void add_values()
{
	// 记录舵机的状态
	// add_value(&duoji_need,sizeof(duoji_need));
}

// 每隔 RUAN_TIMEer_ms
void ruan_timer_ms()
{

	if (user_error_1 == 1)
	{
		if (user_error1.status == NOT_WARN)
		{
			set_warn(&user_error1);
		}
	}
	else
	{
		user_error1.status = NOT_WARN;
	}
	if (user_error_2 == 1)
	{
		if (user_error2.status == NOT_WARN)
		{
			set_warn(&user_error2);
		}
	}
	else
	{
		user_error2.status = NOT_WARN;
	}
}

// 每隔 RUAN_TIMEer_us
void ruan_timer_us()
{
}

// 每隔 RUAN_TIMEer_us
void ruan_timer_1s()
{
}
// 刷新状态
void refresh_work()
{
}

/*此函数在定时中断中调用，处理温湿度传感器的40bit读取*/
void DHT11_read_and_send()
{
	// 读取温湿度，并将异常情况返回
	short t = dht11_read_data(&dht11_data);
	if (t == 0)
	{
		Serial.print("DHT11 error :timeout 超时未回复\r\n");
	}
	else if (t == -1)
	{
		Serial.print("DHT11 error :sum error 数据校验错误\r\n");
	}
	else if (EID > 0)
	{
	}
}

/*	此函数在定时中断中调用，处理温湿度传感器通讯协议中18ms下拉	*/
void read_dht11()
{
	// 让DHT11的信号引脚拉低，等待20ms，之后调用get_DHT11_DATA() 开始正式调用读取函数
	set_timer1_ms(timer1_worker, TIMER1_timeout_ms - LOW_PIN_ms); // 正常时间之后恢复 timer1_worker 的工作
	DHT11_read_and_send();
}
/*每隔 DHT11_SPACE_OF_TIME_ms 读取DHT11*/
int dht11_get()
{
	static short timer2_count = TIMER2_COUNT; //
	timer2_count++;
	if (timer2_count >= TIMER2_COUNT)
	{
		// 先拉低，LOW_PIN_ms 之后调用读取函数
		set_timer1_ms(read_dht11, (unsigned int)dht11_read_ready());
		timer2_count = 0;
		return 0;
	}
	return 1;
}
