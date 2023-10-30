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
extern "C"
{
#include "mystr.h"
#include "mytimer.h"
}

char WIFI_ssid[WIFI_SSID_LEN] = {'\0'};
char WIFI_password[WIFI_PASSWORD_LEN] = {'\0'};
static u64 UID = 0;
uint32_t CHIP_ID = 0;
struct Tcp_cache my_tcp_cache;	  // TCP缓存数组
char tcp_send_data[MAX_TCP_DATA]; // 随用随清，不设置长度数组

/*
目前问题/想法
1	udp发包问题
2	储存一些可以控制的设备的默认启动状态设置，在文件系统里
3	添加低功耗状态，允许用户选择设备是否开启低功耗状态。
	开机后向服务器请求 自己的状态，然后休眠最长时间，或者开机工作。
	重复上一步骤
4	是否可以强制唤醒？

关于心跳包被误认为指令返回值的解决方案
原先时候：	单片机：tcp返回的打包函数 set_databack 会将第一个字节首先填充为 '#'
			手机端：对其进行解析（split("#+")）的时候会将连续的 ‘#’ 识别为一段。
			服务器：先丢包，发指令，转发结果
预期方案：	单片机：set_databack 添加输入参数，分辨心跳包和指令返回包
					在原先的 set_databack 基础之上，所有的数据向后移动一位，第一位用来填充服务器判定标志位
					心跳包：'\t' //'\0'   指令包：'#'	//stm32采用at指令控制esp8266联网时无法发送 '\0',所以决定使用'\t'替换'\0'
			手机端代码不用改
			服务端：舍弃数据的环节放在标志判定为之后，仅对'#'开头的数据进行转发

//为数据命名，设定个数
//关于LED的状态，在与用户的手机沟通期间 1认为电灯属于开启状态 0认为电灯关闭，
//由于服务器将1设为亮灯状态，所以必须确保安装设备完成之后，必须确保手机上显示的状态和电灯的状态一样
//可以修改的位置：
1	继电器引脚
	一共有三个脚，可以控制电灯的状态，应确保 继电器 上电 前后 电灯的状态是相同的（除非从文件系统读取了最后的状态）。
2	修改程序逻辑的高低电平 和 01 的对应关系，方便设备的安装。
*/

/*将数据放在一个数组里发送。 返回数据的长度*/
int set_databack(const char fig, char *tcp_send_data, int max_len)
{
	int i, k, count_char, tmp;
	tcp_send_data[0] = fig; // 在这里插入开始符号
	tcp_send_data[1] = '#'; // 在这里插入开始符号
	count_char = 2;
	for (i = 0; i < MAX_NAME; i++)
	{
		k = 0;
		if (data_list[i].name == NULL)
		{
			break;
		}
		tmp = get_name_str(data_list + i, tcp_send_data + count_char, max_len - count_char);
		if (tmp == -1)
		{
			Serial.printf("get_name_str error -1  i=%d  \r\n", i);
			tcp_send_data[count_char++] = '#'; // 在这里插入单个数据结束符
			continue;
		}
		count_char = count_char + tmp;
		// while (data_list[i].name[k] != '\0') //把数据的名字填充到数组里
		// {
		// 	tcp_send_data[count_char] = data_list[i].name[k];
		// 	k++;
		// 	count_char++;
		// }
		tcp_send_data[count_char++] = ':'; // 在这里插入分隔符
		// char** str_data_names = { "温度" ,"湿度","灯0" ,"灯1" };
		tmp = get_data_unit_str(data_list + i, tcp_send_data + count_char, max_len - count_char);
		if (tmp == -1)
		{
			Serial.printf("get_data_unit_str error -1  i=%d  \r\n", i);
			tcp_send_data[count_char++] = '#'; // 在这里插入单个数据结束符
			continue;
		}
		count_char = count_char + tmp;
		tcp_send_data[count_char++] = '#'; // 在这里插入单个数据结束符
	}
	// Serial.printf("  count_char :%d  ", count_char);
	return count_char;
}

/*
处理服务器发送来的数据，一次只能处理一种类型的数据/指令，根据数据的开头符号，判断数据的指令内容
返回值：0 成功
		1 成功，但是不刷新发送数据包的时间
		101 发送返回给服务器的tcp数据包失败
*/
int do_tcp_data(struct Tcp_cache my_tcp_cache)
{
	const int kERROR_send_tcp = 101;
	const int kMESSAGE_send_tcp_time_no_flush = 1;
	short stat;
	int tcp_senddata_len = 0;
	unsigned long long tmpuL = 0;
	short len_old;
	// Serial.printf("回复响应时间：%d \n", micros() - time_old);
	// 如果需要对TCP链接返回的数据进行处理，请在这后面写，-----------------------------------------------------------------------
	// 示例//将TCP返回的数据当作字符串输出
	Serial.print(my_tcp_cache.data);

	switch (*my_tcp_cache.data)
	{
	case 'A'://新增一个联动监听
		// 尝试使用 '\t' 作为分割符号，一次接收多个监听指令
		tcp_senddata_len = 0;
		int tmp_status = 0;
		tmp1 = str1_find_char_1(my_tcp_cache.data, len_old, my_tcp_cache.len, '\t');
		while (tmp1 > 0)
		{
			my_tcp_cache.data[tmp1] = '\0';
			tmp_status = add_jiantin(my_tcp_cache.data + len_old, tmp1);
			tcp_senddata_len = tcp_senddata_len + sprintf(tcp_send_data + tcp_senddata_len, "L%d", tmp_status);
			len_old = tmp1 + 1;
			tmp1 = str1_find_char_1(my_tcp_cache.data, len_old, my_tcp_cache.len, '\t');
		}
		jiantin_print();
		if (back_send_tcp_(&client, tcp_send_data, tcp_senddata_len) == -1)
		{
			return kERROR_send_tcp;
		}
		send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳
		break;
	case 'C':
		// 重新触发一些监听的事件。好像是把错误状态重置成未触发，清除掉旧的已发送的状态。
		// len_old=0
		tmp1 = 0;
		do
		{
			tmpuL = str_to_u64(my_tcp_cache.data + len_old, my_tcp_cache.len - len_old, &stat);
			len_old = str1_find_char_1(my_tcp_cache.data, len_old + 1, my_tcp_cache.len, 'C');
			if (stat != 1)
			{
				break;
			}
			set_not_warn(tmpuL);
			tmp1 = tmp1 + 1;
		} while (len_old > 0);
		tcp_senddata_len = sprintf(tcp_send_data, "#%d", tmp1);
		if (back_send_tcp_(&client, tcp_send_data, tcp_senddata_len) == -1)
		{
			return;
		}
		send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳
		break;
	case 'D'://删除一个联动
		tmpuL = str_to_u64(my_tcp_cache.data, my_tcp_cache.len, &stat);
		if (stat != 1)
		{
			Serial.printf(" D   str_to_u64 error	%d", stat);
			break;
		}
		jiantin_del(tmpuL);
		tcp_senddata_len = sprintf(tcp_send_data, "%s", my_tcp_cache.data);
		if (back_send_tcp_(&client, tcp_send_data, tcp_senddata_len) == -1)
		{
			return kERROR_send_tcp;
		}
		send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳

		break;
	case 'm'://message 
	case 'w'://warning
	case 'e'://error
		// 这里是服务器收到udp消息之后的回复
		tmpuL = str_to_u64(my_tcp_cache.data, my_tcp_cache.len, &stat);
		if (stat < 0)
		{
			Serial.printf("  mwe  str_to_u64 error	%d", stat);
			break;
		}
		tcp_senddata_len = warn_ack(tmpuL, (enum UdpMessageClass) * (my_tcp_cache.data + len_old), tcp_send_data); // tmp原来存错误id现在存长度
		if (tcp_senddata_len < 2)																				   // 基础长度两个#号
		{
			// 请求的错误已经消除
			Serial.printf("   loop warn_ack return 0 ");
			break;
		}
		if (back_send_tcp_(&client, tcp_send_data, tcp_senddata_len) == -1)
		{
			return kERROR_send_tcp;
		}
		send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳
		break;
	case 'T':
		// case 't':
		// 这里处理心跳包返回的时间戳，无需返回任何数据
		str_get_time(&Now, my_tcp_cache.data);
		kMESSAGE_send_tcp_time_no_flush
		break;
	case '+': // 获取传感器和模式的信息
	case 'G':
	case 'g':
		if (back_send_tcp_(&client, tcp_send_data, set_databack(COMMAND_FIG, tcp_send_data, MAX_TCP_DATA)) == -1)
		{
			return kERROR_send_tcp;
		}
		send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳
		break;						 // 跳出 switch
	case 'I':						 // 获取一些模式id的详细描述
	case 'i'://发送 可选变量描述 MODE_INFO
		if (MODE_INFO == NULL)
		{
			break;
		}
		if (back_send_tcp(&client, MODE_INFO) == -1)
		{
			return kERROR_send_tcp;
		}
		send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳
		break;						 // 跳出 switch
	case '@':						 // set
		while (len_old >= 0 && len_old < my_tcp_cache.len)
		{
			// 计算查找名字数据分割线的范围，取最小值
			tmp1 = len_old + 50;
			if (my_tcp_cache.len < tmp1)
			{
				tmp1 = my_tcp_cache.len;
			}
			// 查找名字数据之间的分割符号
			int value = str1_find_char_1(my_tcp_cache.data, len_old, tmp1, '['); // 获取 '[' 相对于 my_tcp_cache.data 的位置
			if (value < 0)														 // 限制名字的长度,找不到 '[' 就去找 ':'
			{
				// Serial.printf("get '[' error value= %d %d\n", value, len_old);
				value = str1_find_char_1(my_tcp_cache.data, len_old, tmp1, ':'); // 获取':'相对于 my_tcp_cache.data 的位置
			}
			if (value < 0) //
			{
				// Serial.printf("get ':' error value= %d \n", value);
				break;
			}

			for (short i = 0; i < MAX_NAME; i++)
			{
				if (data_list[i].name == NULL)
				{
					break;
				}
				if (1 == str1_eq_str2(my_tcp_cache.data, len_old, value, data_list[i].name))
				{
					value = str1_find_char_1(my_tcp_cache.data, value, my_tcp_cache.len, ':') + 1; // 找到的是 ':' 真正的数据从下一位开始

					if (set_value(data_list + i, my_tcp_cache.data + value, my_tcp_cache.len - value) == 1)
					{
						Serial.printf("set_value ok %d	", i);
					}
					else
					{
						Serial.printf("set_value error %d %s\r\n", i, my_tcp_cache.data + value);
					}
					break;
				}
			}
			my_tcp_cache.data[len_old] = 0; // 清楚标志位的数据
			len_old = str1_find_char_1(my_tcp_cache.data, len_old + 1, my_tcp_cache.len, '@');
			// 只识别 @ 类型的数据，get类型的数据一般不会组合发送，舍弃此部分
		}
		// 所有的指令已经执行完毕
		refresh_work(); // 更新一下光控灯的状态
		// TCP 打包返还自己的状态
		if (back_send_tcp_(&client, tcp_send_data, set_databack(COMMAND_FIG, tcp_send_data, MAX_TCP_DATA)) == -1)
		{
			return kERROR_send_tcp;
		}
		send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳
		if (power_save != 0)		 // 实时更新断电记忆的东西
		{
			// file_save_stut();
			Serial.printf(" save_values %d \r\n", save_values(stut_data_file));
			if (power_save == 1)
			{
				power_save = 0; // 如果不清零，则每次更改设置都会被flash记忆，flash擦写能力有限，我调试程序的就不每次擦写了
			}
		}
		break; // 跳出 switch
	}
	my_tcp_cache.data[0] = 0;
	my_tcp_cache.len = 0; // 因为我没有在其他位置调用数据接收函数，所以我处理完之后全部清除了
}

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
	Serial.printf("star%d", stat);
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
	long long tmp;
	unsigned long long tmpuL = 0;
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
					if (back_send_tcp_(&client, tcp_send_data, set_databack(HEART_BEAT_FIG, tcp_send_data, MAX_TCP_DATA)) == -1)
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
		stat = timeout_back_us(&client, RUAN_TIMEer_us); // 等待100us tcp是否有数据返回

		if (stat == 1) // 有收到TCP数据
		{
			len_old = my_tcp_cache.len; // my_tcp_cache.len=0
			stat = get_tcp_data(&client, &my_tcp_cache);
			if (my_tcp_cache.len < 1) // 收到有效字节
			{
				continue; // 没有收到有效的数据，不用继续往后
			}
			get_time_old_ms = millis(); // 更新最后一次接收到数据的时间戳
			int error = do_tcp_data(my_tcp_cache);
		}
		// TCP链接失效
		else if (stat == 0)
		{
			Serial.printf(" 3 error_tcp_sum=%d \r\n", error_tcp_sum++);
			delay(1000);
			return;
		}
	}

	// Serial.printf(" never  error\r\n");//TCP 刚好失效的时候就触发了
	// client.stop();
}
