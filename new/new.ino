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
#include "usermain.h"
#include "mywarn.h"
#include "mytype.h"
#include "jiantin.h"
#include "DoServerMessage.h"
extern "C"
{
#include "mystr.h"
#include "mytimer.h"
}

char WIFI_ssid[WIFI_SSID_LEN] = {'\0'};
char WIFI_password[WIFI_PASSWORD_LEN] = {'\0'};
static u64 UID = 0;
uint32_t CHIP_ID = 0;

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
		tcp_server_get_wifi_data(WIFI_ssid, WIFI_password, UID, CHIP_ID, wifi_ssid_pw_file);
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
	Serial.printf("#WIFI_ssid:%s  WIFI_password:%s  UID:%ld \r\n", WIFI_ssid, WIFI_password, UID);

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
	WiFiClient client;
	short stat;
	if (client.connect(MYHOST, TCP_PORT))
	{
		if (UID != 0)
		{
			client.printf("+UID:%llu", UID);
		}

		String str1 = "class_id=1,chip_id=" + String(CHIP_ID) + ",ip=" + WiFi.localIP().toString() + ",mac=" + WiFi.macAddress();
		client.print(str1);
		error_tcp_sum = 0;
	}
	else
	{
		Serial.printf(" 1 error_tcp_sum=%d \r\n", error_tcp_sum++);
		delay(1000);		   // 等待  S 再重连
		if (error_tcp_sum > 3) // && error_tcp_sum < 6)
		{
			Serial.print("WiFi.mode(WIFI_OFF);\r\n");
			WiFi.mode(WIFI_OFF); // 重新连接wifi
		}
		return;
	}
	Serial.print("tcp ok");

	stat = timeout_back_ms(&client, 3000);
	if (stat == 1)
	{
		// 这里收到的信息可能是服务器返回的第一条信息
		Serial.println(my_tcp_cache.data + get_tcp_data(&client, &my_tcp_cache));
		// tmp1 临时储存一下+EID的开始位置
		tmp1 = str1_find_str2_(my_tcp_cache.data, my_tcp_cache.len, "+EID");
		if (tmp1 >= 0)
		{
			EID = str_to_u64(my_tcp_cache.data + tmp1, my_tcp_cache.len, &stat);
			if (stat != 1)
			{
				// 值转换出错，溢出或未找到有效值//这种可能是服务器数据错误，休眠一会儿等服务器修复
				Serial.print("\r\nnot found eid,deepSleep\r\n");
				ESP.deepSleep(20000000, WAKE_RFCAL);
			}

			// 从 my_tcp_cache.data 剩下的数据里提取出时间，校准单片机的时间
			tmp1 = str1_find_char_(my_tcp_cache.data, my_tcp_cache.len, 'T');
			// Serial.println(my_tcp_cache.data + tmp1);
			str_get_time(&Now, my_tcp_cache.data + tmp1);

			char str_EID[22] = {0};
			sprintf(str_EID, "%llu", EID); // 垃圾string(),居然不能装换long long类型的数据，还要我自己动手
			UDP_head_data = "+EID=" + String(str_EID) + ",chip_id=" + String(CHIP_ID);
			Serial.print(UDP_head_data);
			error_wifi_count = 0;//这个wifi是有效的，清除错误数量计数
		}
		my_tcp_cache.len = 0;
	}
	else if (stat == 0)
	{
		// 这种可能是服务器数据错误，休眠一会儿等服务器修复
		Serial.print("\r\nservier error,deepSleep\r\n");
		ESP.deepSleep(20000000, WAKE_RFCAL);
		// return;
	}

	unsigned long get_time_old_ms = millis();
	unsigned long send_time_old_ms = millis();
	unsigned long ruan_time_old_ms = millis();
	unsigned long ruan_time_old_1s = millis();

	// micros();//us
	refresh_work(); // 更新一下状态
	// 用户初始化
	my_init();

	while (client.connected())
	{
		/*关于00：00断网
		之前的理解	//tcp断开之后无法重新链接，我只能重新声明试试，但是好像也没什么用处???，只能计次，然后软件复位程序
		现在的理解	2021年5月22日11点54分	tcp链接由于 校园网日租网络租约结束 而断开，无法完成数据通讯，过20min左右才会被系统感知发现，然后 client.connected() 判定才为假
					在这之间的20min里，发送的数据均无法到达服务器，链接实际上是断开的状态，似乎是是由于没有收到tcp的挥手，链接判定值为真
		解决方案	依靠更合理的心跳包。心跳包就是心跳包，没有按时收到消息就是暴毙了。
			每隔1min发送一次tcp心跳包，发送后2min内收到回复视为正常		//考虑到服务器在以后可能收到很多设备的心跳包，处理起来占用很多时间，将这个搞成动态时间间隔？
			如果没有收到心跳包的回复就开始重新建立链接	这里直接 return 就可以了
		实现情况	无
		*/
		// tcp链接发送数据时候过将近20min才会被系统感知发现，然后 client.connected() 判定才为假

		// 声音的采样间隔，查看 ruan_time_old_ms 时间间隔内的高电平数量，作为声控的判定标准
		if (millis() - ruan_time_old_ms > RUAN_TIMEer_ms)
		{
			if (millis() - ruan_time_old_1s > 1000)
			{
				ruan_time_old_1s = millis();
				next_sec(&Now);
				ruan_timer_1s();

				// 隔一段时间就发送一次本机数据，心跳包
				if (millis() - send_time_old_ms > HEART_BEAT_ms) // 无符号整型的加减运算，就算溢出了，也不影响差的计算，windows gcc 0-0xfffffffe=2;
				{
					if (millis() - get_time_old_ms > HEART_BEAT_TIMEOUT_ms)
					{
						// 太久没有收到服务器发来的数据，重新连接服务器
						Serial.printf(" %d not get tcp data ,return\r\n", HEART_BEAT_TIMEOUT_ms);
						return;
					}
					Serial.print('#');
					// TCP发送心跳包
					if (send_hart_back(&client) == -1)
					{
						return;
					}
					// 在这里插入心跳包的返回值检测，超时未回复，就认定链接失效，重新建立tcp的链接
					/*实现方式：添加两个标志？记录发送状态和接收状态，判断发送和接受状态的情况*/
					send_time_old_ms = millis();
				}
			}

			// 验证监听数据
			if (jiantin_loop() < 0)
			{
				Serial.printf("   jiantin_loop error\r\n");
			}
			// 发送警告
			warn_send();
			// 调用软定时器
			ruan_timer_ms();			 // 每隔 RUAN_TIMEer_ms
			ruan_time_old_ms = millis(); // 更新时间
		}
		ruan_timer_us();								 // 每隔 RUAN_TIMEer_us


		stat = wait_and_do_server_message(&client,&send_time_old_ms);
		if(stat == 101){
			Serial.printf("error: file %s,line %d, error_tcp_sum %d\r\n",__FILE__,__LINE__,error_tcp_sum++);
			delay(1000);
			return;
		}else if(stat == 1){
			get_time_old_ms = millis(); // 更新最后一次接收到数据的时间戳
		}
	}

	// Serial.printf(" never  error\r\n");//TCP 刚好失效的时候就触发了
	// client.stop();
}
