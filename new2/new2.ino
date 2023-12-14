#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LittleFS.h>
#include "src/myinclude.h"

uint32_t CHIP_ID = 0;

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 旧的usermain移过来的  ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

const char *MODE_INFO = "@断电记忆[0-2]:关闭，仅本次，所有";
static struct Udpwarn user_error1 = {WARN, NOT_WARN, 0, 5, "1 号自定义警告被触发"};
static struct Udpwarn user_error2 = {WARN, NOT_WARN, 0, 6, "2 号自定义警告被触发"};

const char *sec_timer2_name = "@1定时器";
const char *min_timer1_name = "@2定时器";

// 定义传感器储存变量
uint8_t power_save = 0; // 断电记忆
uint8_t user_error_1 = 0;
uint8_t user_error_2 = 0;
double yan_wu_A = 0;
double yanwu_my = 0;
uint8_t jidianqi_value = 0;
uint8_t jidianqi_value1 = 0;
uint8_t yu_men[4 * 2 + 3 * 2] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 与门寄存器

int8_t start_sec_timer_1 = -1;	 // 秒
int8_t next_minute_timer_1 = -1; // 分
int32_t min_timer_1 = -1;

unsigned long timer_2_last_ms;
int32_t sec_timer_2 = -1;

unsigned long key_down_start_time = 0;
unsigned long key_up_start_time = 0;

unsigned long key_left_down_start_time = 0;
unsigned long key_right_down_start_time = 0;

int8_t key_left_in_last_down = 0;
int8_t key_right_in_last_down = 0;

// 定义几个引脚的功能
const uint8_t anjian1 = 0; // 按键1输入

const uint8_t anjian_pin_left = 13; //
const uint8_t anjian_pin_right = 2; //

const uint8_t dht11 = 5;		  // dht11
const uint8_t jidianqi_pin = 12;  // 继电器
const uint8_t jidianqi_pin1 = 14; // 继电器

// 定义几个范围定义变量
unsigned char CONST1[4] = {0, 1, 2, 6};
int32_t CONST2[4] = {0, 180};
// 函数声明
void refresh_work();

struct MyType data_list[MAX_NAME] = {
	{"烟雾模拟", "%", TYPE_DOUBLE, sizeof(yanwu_my), &yanwu_my, NULL, NULL},
	{"@1号自定义警告", NULL, TYPE_u8, sizeof(user_error_1), &user_error_1, CONST1, CONST1 + 1}, // 用户自定义警告
	{"@2号自定义警告", NULL, TYPE_u8, sizeof(user_error_2), &user_error_2, CONST1, CONST1 + 1}, // 用户自定义警告
	{"@断电记忆", NULL, TYPE_u8, sizeof(power_save), &power_save, CONST1, CONST1 + 2},
	{"@1与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men, CONST1, CONST1 + 1},		// 1号与门1号入口
	{"@1与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 1, CONST1, CONST1 + 1},	// 1号与门2号入口
	{"@1与3入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 2, CONST1, CONST1 + 1},	// 1号与门3号入口
	{"1与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 3, CONST1, CONST1 + 1},	// 1号与门输出
	{"@2与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 4, CONST1, CONST1 + 1},	// 2号与门1号入口
	{"@2与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 5, CONST1, CONST1 + 1},	// 2号与门2号入口
	{"@2与3入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 6, CONST1, CONST1 + 1},	// 2号与门3号入口
	{"2与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 7, CONST1, CONST1 + 1},	// 2号与门输出
	{"@3与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 8, CONST1, CONST1 + 1},	// 3号与门1号入口
	{"@3与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 9, CONST1, CONST1 + 1},	// 3号与门2号入口
	{"3与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 10, CONST1, CONST1 + 1},	// 3号与门输出
	{"@4与1入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 11, CONST1, CONST1 + 1}, // 4号与门1号入口
	{"@4与2入", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 12, CONST1, CONST1 + 1}, // 4号与门2号入口
	{"4与出", NULL, TYPE_u8, sizeof(yu_men[0]), yu_men + 13, CONST1, CONST1 + 1},	// 4号与门输出
	{"时", NULL, TYPE_u8, sizeof(Now.hour), &(Now.hour), NULL, NULL},
	{"分", NULL, TYPE_u8, sizeof(Now.minute), &(Now.minute), NULL, NULL},
	{"秒", NULL, TYPE_u8, sizeof(Now.sec), &(Now.sec), NULL, NULL},
	{"星期", NULL, TYPE_u8, sizeof(Now.week), &(Now.week), CONST1, CONST1 + 3},
	{"日", NULL, TYPE_u8, sizeof(Now.day), &(Now.day), NULL, NULL},
	{"月", NULL, TYPE_u8, sizeof(Now.month), &(Now.month), NULL, NULL},
	{"年", NULL, TYPE_USHORT, sizeof(Now.year), &(Now.year), NULL, NULL},
	{NULL} // 到这里结束

};

void timer1_worker()
{
	// 长按按键1清除wifi账号密码记录
	clear_wifi_data(wifi_ssid_pw_file);

	yanwu_my = system_adc_read() * 100 / 1024.00;
}

/*请注意引脚初始化和赋值的先后顺序，不然可能导致外接的继电器闪烁
 */
void my_init()
{
	refresh_yu_men();
	refresh_jidianqi(); // 初始化引脚之前，先调整高低电平，减少不必要的继电器响声
	pinMode(anjian1, INPUT); // 按键1
	pinMode(jidianqi_pin, OUTPUT);
	pinMode(jidianqi_pin1, OUTPUT);
	pinMode(anjian_pin_left, INPUT);
	// pinMode(jd1, OUTPUT);
	set_timer1_ms(timer1_worker, TIMER1_timeout_ms); // 强制重新初始化定时中断，如果单纯的使用 dht11_get 里的过程初始化，有概率初始化失败
	// （仅在程序复位的时候可以成功，原因：timer2_count 没有复位就不会被初始化，自然调用不到定时器的初始化函数），
}

// 需要断电记忆的变量在这里添加
void add_values()
{
	// 记录舵机的状态
	add_value(&jidianqi_value, sizeof(jidianqi_value));
	add_value(&jidianqi_value1, sizeof(jidianqi_value1));
}
void refresh_jidianqi()
{
	if (jidianqi_value == 1)
	{
		digitalWrite(jidianqi_pin, LOW);
	}
	else
	{
		digitalWrite(jidianqi_pin, HIGH);
	}
	if (jidianqi_value1 == 1)
	{
		digitalWrite(jidianqi_pin1, LOW);
	}
	else
	{
		digitalWrite(jidianqi_pin1, HIGH);
	}
}
void refresh_yu_men()
{
	yu_men[3] = yu_men[0] & yu_men[1] & yu_men[2];
	yu_men[7] = yu_men[4] & yu_men[5] & yu_men[6];
	yu_men[10] = yu_men[8] & yu_men[9];
	yu_men[13] = yu_men[11] & yu_men[12];
}

/*

*/
void refresh_min_timer_1()
{
	if (min_timer_1 == 0)
	{
		if (Now.sec >= start_sec_timer_1)
		{
			min_timer_1 = -1;
			start_sec_timer_1 = -1;
			next_minute_timer_1 = -1;
		}
	}
	else if (min_timer_1 > 0)
	{
		if (Now.minute >= next_minute_timer_1)
		{
			min_timer_1--;
			next_minute_timer_1 = Now.minute + 1;
			if (next_minute_timer_1 == 60)
			{
				next_minute_timer_1 = 0;
			}
		}
	}
}
void refresh_sec_timer_2()
{
	if (sec_timer_2 < 0)
	{
		return;
	}
	if (millis() - timer_2_last_ms >= 1000)
	{
		sec_timer_2--;
		timer_2_last_ms = millis();
	}
}

void refresh_key_down()
{

	if (digitalRead(anjian1) == HIGH)
	{
		key_down_start_time = millis();
	}
	else if (digitalRead(anjian1) == LOW && millis() - key_down_start_time > 50)
	{
		if (jidianqi_value1 == 1 || jidianqi_value == 1)
		{
			jidianqi_value1 = 0;
			jidianqi_value = 0;
			refresh_jidianqi();
		}
		key_up_start_time = millis();
		return;
	}
	if (millis() - key_up_start_time < TIMER1_timeout_ms * 2)
	{
		return;
	}

	pinMode(anjian_pin_left, INPUT);
	if (digitalRead(anjian_pin_left) == HIGH)
	{
		key_left_down_start_time = millis();
		key_left_in_last_down = 0;
	}
	else if (key_left_in_last_down == 0 && millis() - key_left_down_start_time > 50)
	{
		if (jidianqi_value1 == 1)
		{
			jidianqi_value1 = 0;
		}
		else
		{
			jidianqi_value1 = 1;
		}
		refresh_jidianqi();
		key_left_in_last_down = 1;
	}

	pinMode(anjian_pin_right, INPUT);
	if (digitalRead(anjian_pin_right) == HIGH)
	{
		key_right_down_start_time = millis();
		key_right_in_last_down = 0;
	}
	else if (key_right_in_last_down == 0 && millis() - key_right_down_start_time > 50)
	{
		if (jidianqi_value == 1)
		{
			jidianqi_value = 0;
		}
		else
		{
			jidianqi_value = 1;
		}
		refresh_jidianqi();
		key_right_in_last_down = 1;
	}
	pinMode(anjian_pin_right, OUTPUT);
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
	refresh_key_down();
	refresh_yu_men();
	refresh_jidianqi();
	refresh_min_timer_1();
	refresh_sec_timer_2();
}

void set_value_of_data_list(int index)
{
	if (data_list[index].name == min_timer1_name)
	{
		// 记录分钟级别的定时器开始的秒，和刷新一下下次减一的位置
		next_minute_timer_1 = Now.minute + 1;
		if (next_minute_timer_1 == 60)
		{
			next_minute_timer_1 = 0;
		}
		start_sec_timer_1 = Now.sec;
	}
	else if (data_list[index].name == sec_timer2_name)
	{
		timer_2_last_ms = millis();
	}
}

void net_set_value_callback()
{
	refresh_yu_men();
	refresh_jidianqi();
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

/*
0 超时
1 建立失败

*/
int try_tcp_loop()
{
	struct TcpLinkData tcp_link_data = init_server_tcp_link(MYHOST, TCP_PORT, get_user_id(), ESP.getChipId());
	if (!tcp_link_data.client)
	{
		Serial.printf("error: file %s,line %d,\r\n", __FILE__, __LINE__);
		return 1;
	}

	// 用户初始化
	my_init();

	int stat;
	stat = wait_and_do_server_message(&tcp_link_data, net_set_value_callback, set_value_of_data_list);
	while (stat != 101)
	{
		stat = wait_and_do_server_message(&tcp_link_data, net_set_value_callback, set_value_of_data_list);
		/* code */
		user_loop_1();
	}

	Serial.printf("error: file %s,line %d,\r\n", __FILE__, __LINE__);
	free_tcp_lick(&tcp_link_data);
	return 0;
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
	do
	{
		if (try_tcp_loop() == 1)
		{
			error_tcp_sum++;
		}
		else
		{
			error_tcp_sum = 0;
		}
		Serial.printf("error: file %s,line %d, error_tcp_sum %d\r\n", __FILE__, __LINE__, error_tcp_sum);
		delay(1000);
	} while (error_tcp_sum > 3);

	// Serial.printf(" never  error\r\n");//TCP 刚好失效的时候就触发了
	// client.stop();
}