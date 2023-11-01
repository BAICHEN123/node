#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LittleFS.h>
#include "test.h"
#include "savevalues.h"
#include "mywifi.h"
#include "mytcp.h"
#include "myconstant.h"
#include "myudp.h"
#include "mywarn.h"
#include "mytype.h"
#include "jiantin.h"
#include "DoServerMessage.h"
extern "C"
{
#include "mystr.h"
#include "mytimer.h"
#include "DHT11.h"
}

uint32_t CHIP_ID = 0;


// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 旧的usermain移过来的  ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

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
	// refresh_work(); // 初始化引脚之前，先调整高低电平，减少不必要的继电器响声

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
void refresh_work()
{

}
void user_loop_1()
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


// ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑ 旧的usermain移过来的   ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑








void setup()
{
	pinMode(16, OUTPUT);
	digitalWrite(16, HIGH); // 模块的初始状态是LOW，然后我一插上跳线帽就开始无限重启//是电压低的问题，默认未初始化的电压低于判定电压，在 nodemcu 上没有出现错误可能是因为产品型号/批次的不同，经过电压表测量，nodemcu的电压在0.8V左右，单个小模块的电压不到0.3
	pinMode(0, INPUT);		// 按键1
	add_values();			// 挂载读取信息。//这里可以优化，仅在读取写入的时候使用数组，建立//但是也没多大用，一个不超过50字节的数组
	set_anjian1(0);			// 配置wifi的清除数据按键

	// LittleFS.format();//第一次使用flash需要将flash格式化,可以不显式调用，会自己初始化

	Serial.begin(115200);
	// CHIP_ID = ESP.getFlashChipId();
	// Serial.printf("unsigned long %d \n", sizeof(unsigned long)); //这个id是假的，不知道为啥，两个esp的一样
	// Serial.printf("long long %d  \n", sizeof(long long));
	CHIP_ID = ESP.getChipId();
	Serial.printf("getFlashChipId %d \r\n", ESP.getFlashChipId()); // 这个id是假的，不知道为啥，两个esp的一样
	Serial.printf("getChipId %d  \r\n", ESP.getChipId());

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	short stat = file_read_wifidata(WIFI_ssid, WIFI_password, wifi_ssid_pw_file);
	Serial.printf("file_read_wifidata stat %d ", stat);
	if (stat == -1)
	{
		digitalWrite(LED_BUILTIN, LOW);
		tcp_server_get_wifi_data(WIFI_ssid, WIFI_password, ESP.getChipId(), wifi_ssid_pw_file);
		digitalWrite(LED_BUILTIN, HIGH);
	}
	else if (stat == -2)
	{
		// 文件系统错误，基本不会发生，程序调试或者产品验证的时候就能发现并排除错误
		Serial.print("flash error ,file open error -2,deepSleep");
		ESP.deepSleep(20000000, WAKE_RFCAL);
	}

	// 简单验证下是否读取到有效的wifi数据
	if (WIFI_password[0] == '\0' || WIFI_ssid[0] == '\0')
	{
		Serial.println("delete & restart");
		file_delete(wifi_ssid_pw_file);
		ESP.restart(); // 这个函数在串口打印的开机信息里是看门狗复位
	}
	Serial.printf("#WIFI_ssid:%s  WIFI_password:%s  UID:%ld \r\n", WIFI_ssid, WIFI_password, get_user_id());

	// 连接wifi
	Serial.printf("CHIP_ID %x \r\n", CHIP_ID);
	if (get_wifi(WIFI_ssid, WIFI_password, wifi_ssid_pw_file) == 0)
	{
		// 无法连接WiFi，休眠一会儿。//WiFi修复是很慢的事情，也有可能是密码改了，可以通过复位键立刻唤醒
		Serial.print("wifi error 0,deepSleep");
		ESP.deepSleep(20000000, WAKE_RFCAL);
	}

	// Serial.printf(" file_read_stut %d ", file_read_stut());
	stat = read_values(stut_data_file);
	Serial.printf(" read_values %d \r\n", stat);
	if (stat == -1)
	{
		// 文件储存的内容过期
		file_delete(stut_data_file);
		ESP.restart();
	}
}

void loop()
{
	static short error_wifi_count = 0;
	static int error_tcp_sum = 0; // tcp链接失败计次
	int tmp1;
	timer1_disable();

	if (WiFi.status() != WL_CONNECTED)
	{
		// 计次，超过三次就休眠
		if (get_wifi(WIFI_ssid, WIFI_password, wifi_ssid_pw_file) == 0)
		{
			Serial.printf("error_wifi_count==%d\r\n", error_wifi_count);
			if (error_wifi_count == 1)
			{
				Serial.print("WiFi.mode(WIFI_OFF);\r\n");
				WiFi.mode(WIFI_OFF); // 断开wifi 之后重新连接wifi
				delay(1000);
			}
			else if (error_wifi_count == 2)
			{
				Serial.print("\r\nrestart\r\n");
				ESP.restart(); // 这个函数在串口打印的开机信息里是看门狗复位
			}
			error_wifi_count++;
			return;
		}
		// set_timer1_s(realy_DHT11, 2);
	}

	// WiFi.mode(WIFI_OFF); // 重新连接wifi

	struct TcpLinkData tcp_link_data = init_server_tcp_link(MYHOST,TCP_PORT,get_user_id(),ESP.getChipId());
	if(!tcp_link_data.client)
	{
		Serial.printf("error: file %s,line %d, error_tcp_sum %d\r\n",__FILE__,__LINE__,error_tcp_sum++);
		delay(1000);
		return;
	}


	// 用户初始化
	my_init();


	int stat;
	stat = wait_and_do_server_message(&tcp_link_data,refresh_work);
	while (stat!=101)
	{
		stat = wait_and_do_server_message(&tcp_link_data,refresh_work);
		/* code */
		user_loop_1();
	}

	Serial.printf("error: file %s,line %d, error_tcp_sum %d\r\n",__FILE__,__LINE__,error_tcp_sum++);
	free_tcp_lick(&tcp_link_data);
	delay(1000);
	return;

	// Serial.printf(" never  error\r\n");//TCP 刚好失效的时候就触发了
	// client.stop();
}

