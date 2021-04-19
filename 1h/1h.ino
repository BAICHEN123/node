#include <ESP8266WiFi.h>
//#include "FS.h"
#include <stdio.h>
#include <stdlib.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LittleFS.h>
#include "test.h"
extern "C"
{
#include "DHT11.h"
#include "mystr.h"
}
#define TIMER1_timeout_ms 200

const char *wifi_ssid_pw_file = "/wifidata.txt";
const char *stut_data_file = "/stutdata.txt";
const char *MYHOST = "121.89.243.207";
const uint16_t TCP_PORT = 9999;
const uint16_t UDP_PORT = 9998;
#define WIFI_ssid_len 32
#define WIFI_password_len 32
char WIFI_ssid[WIFI_ssid_len] = {'\0'};
char WIFI_password[WIFI_password_len] = {'\0'};
unsigned long UID = 0;
unsigned long EID = 0;
//u64 CHIP_ID=0;
uint32_t CHIP_ID = 0;

#define MAX_TCP_DATA 1024	   //TCP缓存的最大值
#define MAX_UDP_SEND_DATA 1024 //UDP缓存的最大值
char tcp_data[MAX_TCP_DATA];   //TCP缓存数组
short tcp_data_len = 0;		   //TCP数据的临时缓存
//char qstr_sprint[MAX_UDP_SEND_DATA];
String UDP_head_data = "";
String UDP_send_data = "";
char tcp_send_data[MAX_TCP_DATA]; //随用随清，不设置长度数组

/*
目前问题/想法
1	udp发包问题
2	储存一些可以控制的设备的默认启动状态设置，在文件系统里
3	添加低功耗状态，允许用户选择设备是否开启低功耗状态。
	开机后向服务器请求 自己的状态，然后休眠最长时间，或者开机工作。
	重复上一步骤
4	是否可以强制唤醒？

*/
/*
//为数据命名，设定个数
//关于LED的状态，在与用户的手机沟通期间 1认为电灯属于开启状态 0认为电灯关闭，
//由于服务器将1设为亮灯状态，所以必须确保安装设备完成之后，必须确保手机上显示的状态和电灯的状态一样
//可以修改的位置：
1	继电器引脚
	一共有三个脚，可以控制电灯的状态，应确保 继电器 上电 前后 电灯的状态是相同的（除非从文件系统读取了最后的状态）。
2	修改程序逻辑的高低电平 和 01 的对应关系，方便设备的安装。
*/
#define MAX_NAME 13
char *str_data_names[MAX_NAME] = {"温度",
								  "湿度",
								  "亮度",
								  "@开关1[0-1]",
								  "@开关1模式[0-3]",
								  "@开关2[0-1]",
								  "@开关2模式[0-3]",
								  "@声控灯时长/S[1-300]",
								  "声控灯剩余时长/S",
								  "@高温警告/°C[0-40]",
								  "@低温警告/°C[0-40]",
								  "@补光区间[0-10]",
								  "@保存当前为断电记忆[0-1]"};
struct DHT11_data dht11_data = {666, 666};
//两个开关，当他为2时，是自动模式，其他时候读取12 和14号脚的电平
uint8_t LED1 = 0;
uint8_t switch_1 = 2;
uint8_t LED2 = 0;
uint8_t switch_2 = 2;
short switch_2_light_up_TIME_s = 30;  //重新加载的值
short switch_2_light_up_time_x_s = 0; //计数器用
short TEMPERATURE_ERROR_HIGH = 40;
short TEMPERATURE_ERROR_LOW = 10;
uint8_t light_qu_yu = 5; //补光区间
uint8_t power_save = 0;	 //补光区间

//下面定义几个引脚的功能
const uint8_t jd1 = 14;		//1号继电器
const uint8_t jd2 = 12;		//2号继电器
const uint8_t light = 13;	//光敏逻辑输入
const uint8_t shengyin = 4; //声音逻辑输入
const uint8_t anjian1 = 0;	//按键1输入
const uint8_t dht11 = 5;	//按键1输入

//错误计次变量区域
int error_tcp_sum = 0; //tcp链接失败计次
int debug_udp_count = 0;

//复位函数
void (*resetFunc)(void) = 0;

//其他函数声明
void timer2_worker();
void realy_DHT11();
void set_timer1_s(timercallback userFunc, double time_s);
void set_timer1_ms(timercallback userFunc, uint32_t time_ms);

/*
void get_files_name()
{
	Dir dir = LittleFS.openDir("/");  // 建立“目录”对象
	while (dir.next())
	{  // dir.next()用于检查目录中是否还有“下一个文件”
		Serial.println(dir.fileName()); // 输出文件名
	}
	Serial.println("end"); // 输出文件名
}
*/

short file_read_wifidata()
{
	//Serial.println("SPIFFS format start");
	//LittleFS.format();    // 格式化SPIFFS
	//Serial.println("SPIFFS format finish");
	if (LittleFS.begin())
	{ // 启动SPIFFS
		Serial.println("SPIFFS Started.");
	}
	else
	{
		Serial.println("SPIFFS Failed to Start.");
		return -2; //文件系统启动失败
	}
	//确认闪存中是否有file_name文件
	if (LittleFS.exists(wifi_ssid_pw_file))
	{
		Serial.print(wifi_ssid_pw_file);
		Serial.println(" FOUND.");
	}
	else
	{
		Serial.print(wifi_ssid_pw_file);
		Serial.print(" NOT FOUND.");
		return -1; //未找到WiFi文件
	}
	//建立File对象用于从SPIFFS中读取文件
	File dataFile1 = LittleFS.open(wifi_ssid_pw_file, "r");

	/*
	规定  /wifidata.txt 此文件储存WiFi的账号和密码
	第一行储存账号	WIFI_ssid
	第二行储存密码	WIFI_password
	两行都采用\n 作为结束符
	*/

	//读取文件内容并将文件内容WiFi账号和密码写入数组
	int i;
	if (dataFile1.size() > WIFI_ssid_len + WIFI_password_len)
	{
		dataFile1.close(); //推出之前关闭文件，防止未知错误
		return -3;		   //文件长度错误
	}
	for (i = 0; i < dataFile1.size() && i < WIFI_ssid_len; i++)
	{
		WIFI_ssid[i] = (char)dataFile1.read();
		if (WIFI_ssid[i] == '\n' || WIFI_ssid[i] == '\0')
		{
			WIFI_ssid[i] = '\0';
			break;
		}
	}
	for (; i < WIFI_ssid_len; i++)
	{
		WIFI_ssid[i] = '\0';
	}
	for (i = 0; i < dataFile1.size() && i < WIFI_password_len; i++)
	{
		WIFI_password[i] = (char)dataFile1.read();
		if (WIFI_password[i] == '\n' || WIFI_password[i] == '\0')
		{
			WIFI_password[i] = '\0';
			break;
		}
	}
	for (; i < WIFI_password_len; i++)
	{
		WIFI_password[i] = '\0';
	}
	//完成文件读取后关闭文件
	dataFile1.close();
	return 1;
}

short file_save_wifidata()
{
	if (LittleFS.begin())
	{ // 启动SPIFFS
		Serial.println("SPIFFS Started.");
	}
	else
	{
		Serial.println("SPIFFS Failed to Start.");
		return -2; //文件系统启动失败
	}

	//确认闪存中file_name文件是否被清楚
	if (LittleFS.exists(wifi_ssid_pw_file) && LittleFS.remove(wifi_ssid_pw_file))
	{
		Serial.print(wifi_ssid_pw_file);
		Serial.println(" found && delete");
	}
	else
	{
		Serial.print(wifi_ssid_pw_file);
		Serial.print("NOT FOUND");
	}
	/*
	规定  /wifidata.txt 此文件储存WiFi的账号和密码
	第一行储存账号	WIFI_ssid
	第二行储存密码	WIFI_password
	两行都采用\n 作为结束符
	*/
	File dataFile = LittleFS.open(wifi_ssid_pw_file, "w"); // 建立File对象用于向SPIFFS中的file对象（即/notes.txt）写入信息
	dataFile.print(WIFI_ssid);
	dataFile.print('\n');
	dataFile.print(WIFI_password);
	dataFile.print('\n');
	dataFile.close(); // 完成文件写入后关闭文件
	return 1;
}

short file_delete_wifidata()
{
	if (LittleFS.begin())
	{ // 启动SPIFFS
		Serial.println("SPIFFS Started.");
	}
	else
	{
		Serial.println("SPIFFS Failed to Start.");
		return -2; //文件系统启动失败
	}

	//确认闪存中file_name文件是否被清楚
	if (LittleFS.exists(wifi_ssid_pw_file) && LittleFS.remove(wifi_ssid_pw_file))
	{
		Serial.print(wifi_ssid_pw_file);
		Serial.println(" found && delete");
	}
	else
	{
		Serial.print(wifi_ssid_pw_file);
		Serial.print("NOT FOUND");
	}
	return 1;
}

/*
UDP发送函数封装起来，方便调用
返回值为发送结果， 0或1
调用示例 ：UDP_Send(MYHOST, UDP_PORT, "UDP send 汉字测试 !");
*/
/*
short UDP_Send(const char* UDP_IP, uint16_t UDP_port, char* udp_send_data)
{
	if (WiFi.status() != WL_CONNECTED)
	{
		return -1;
	}
	WiFiUDP Udp;
	// send a reply, to the IP address and TCP_PORT that sent us the packet we received
	Udp.beginPacket(UDP_IP, UDP_port);
	Udp.write(udp_send_data);
	return  Udp.endPacket();
}
*/

/*
UDP发送函数封装起来，方便调用
返回值为发送结果， 0或1
调用示例 ：UDP_Send(MYHOST, UDP_PORT, "UDP send 汉字测试 !");
*/
short UDP_Send(const char *UDP_IP, uint16_t UDP_port, String udp_send_data)
{

	if (WiFi.status() != WL_CONNECTED)
	{
		return -1;
	}
	WiFiUDP Udp;
	// send a reply, to the IP address and TCP_PORT that sent us the packet we received
	//Serial.printf("udp send id=%d\n",debug_udp_count++);
	Udp.beginPacket(UDP_IP, UDP_port);
	Udp.write(udp_send_data.c_str());
	return Udp.endPacket();
}

/*
此函数根据光强返回是否需要开灯
1	开灯
0	关灯
*/
uint8_t light_high()
{
	static unsigned long last_time = millis();
	static uint16 brightness = system_adc_read();
	if (last_time - millis() > 10) //限制adc读取频率
	{
		brightness = system_adc_read(); //值越大约黑暗 最高1024
		last_time = millis();
	}
	if (brightness > light_qu_yu * 100)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
此函数根据声音返回是否需要开灯
1	开灯
0	关灯
*/
uint8_t sheng_yin_high()
{
	if (switch_2_light_up_time_x_s > 0)
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
	if (switch_2_light_up_time_x_s > 0)
	{
		if (count == 0)
		{
			count = COUNT;
			switch_2_light_up_time_x_s--;
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
	digitalWrite(pin_x, *LEDx);
}

/*
此函数在定时中断中调用，处理光强传感器
*/
void brightness_work()
{
	set_jdq(jd1, switch_1, &LED1);
	set_jdq(jd2, switch_2, &LED2);
}

/*
此函数在定时中断中调用，处理温湿度传感器的40bit读取
*/
void DHT11_read_and_send()
{
	//读取温湿度，并将异常情况返回
	short t = read_data(&dht11_data);
	if (t == 0)
	{
		Serial.print("DHT11 error :timeout 超时未回复\n");
		//UDP_Send(MYHOST, UDP_PORT, "error:DHT11 timeout_back");//-------------------------------------------------------------这里还要统一通讯协议，当设备的驱动报错的时候需要的信号是什么样子的，当传感器的数据异常的的时候的信号是什么样子的
	}
	else if (t == -1)
	{
		Serial.print("DHT11 error :sum error 数据校验错误\n");
		//UDP_Send(MYHOST, UDP_PORT, "error:DHT11 sum_erroe");
	}
	else if (EID > 0)
	{
		//不知道为啥下面两个字符串爆红，但是编译能过，字符串指针原型是数组
		if (dht11_data.temperature > TEMPERATURE_ERROR_HIGH)
		{
			UDP_Send(MYHOST, UDP_PORT, UDP_head_data + "temperature high");
		}
		else if (dht11_data.temperature < TEMPERATURE_ERROR_LOW)
		{
			UDP_Send(MYHOST, UDP_PORT, UDP_head_data + "temperature low");
		}
	}
}

/*
定时1.5s读取一次DHT11的数据
从dht11_data 里读取数据
携带esp8266的flash的id用UDP发送给MYHOST
*/
void timer2_worker()
{
	//读取一下数据，然后将定时器定在1.5s之后，启动18ms的下拉
	//delay(20);//时间中断函数里不可以用delay
	static short timer2_count = 7; //
	static short count_anjian = 0; //对按键按下的时间计数，超过5s就清除wifidata.txt文件，然后重新启动系统
	timer2_count++;
	//读取光强传感器，并控制继电器

	brightness_work();		//更新继电器状态
	shengyin_timeout_out(); //更新声控灯倒计时

	//每隔大约1-2S读取DHT11
	if (timer2_count == 8)
	{
		DHT11_read_and_send();
		set_timer1_ms(timer2_worker, TIMER1_timeout_ms);
		timer2_count = 0;
	}
	else if (timer2_count == 7)
	{
		set_timer1_ms(realy_DHT11, TIMER1_timeout_ms);
	}

	//长按 25*TIMER1_timeout_ms ms
	//删除之前记住的WiFi账号和密码，然后重新启动系统
	if (digitalRead(anjian1) == LOW)
	{

		digitalWrite(LED_BUILTIN, LOW);
		count_anjian++;
		Serial.printf("-%dS -", (25 - count_anjian) / 5);
		if (count_anjian > 25)
		{
			//删除之前记住的WiFi账号和密码，然后重新启动系统
			Serial.println("delete & restart");
			file_delete_wifidata();
			resetFunc();
		}
	}
	else
	{
		digitalWrite(LED_BUILTIN, HIGH);
		count_anjian = 0;
	}
}

/*
此函数在定时中断中调用，处理温湿度传感器通讯协议中18ms下拉
*/
void realy_DHT11()
{
	//让DHT11的信号引脚拉低，等待20ms，之后调用get_DHT11_DATA() 开始正式调用读取函数
	set_timer1_ms(timer2_worker, (unsigned int)read_ready());
}

/*
TCP阻塞，等待 timeout_ms_max ms
看看有没有TCP包返回回来，不会自动断开链接
返回值
0，TCP失效了
1，等到了TCP包
2，没有等到，超时
*/
short timeout_back(WiFiClient client, unsigned short timeout_ms_max)
{
	unsigned long timeout = millis();
	while (client.available() == 0)
	{
		if (millis() - timeout > timeout_ms_max)
		{
			Serial.println(">>> Client Timeout !");
			return 2;
		}
		if (client.connected() == 0)
		{
			return 0;
		}
	}
	return 1;
}

/*
TCP阻塞，等待 timeout_us_max ms
看看有没有TCP包返回回来，不会自动断开链接
返回值
0，TCP失效了
1，等到了TCP包
2，没有等到，超时
*/
short timeout_back_us(WiFiClient client, unsigned short timeout_us_max)
{
	unsigned long timeout = micros();
	while (client.available() == 0)
	{
		if (micros() - timeout > timeout_us_max)
		{
			//Serial.println(">>> Client Timeout !");
			return 2;
		}
		if (client.connected() == 0)
		{
			return 0;
		}
	}
	return 1;
}

/*
从TCP连接读取TCP数据，储存在 tcp_data 里，长度为tcp_data_len，每个数据会用“\0”分割
返回本次TCP读取的开始位置，
*/
short get_tcp_data(WiFiClient client)
{
	short start_id = tcp_data_len;
	while (client.available() && tcp_data_len < MAX_TCP_DATA)
	{
		if (tcp_data_len >= MAX_TCP_DATA)
			return -1;
		tcp_data[tcp_data_len++] = static_cast<char>(client.read());
	}
	tcp_data[tcp_data_len] = '\0';
	return start_id;
}

/*
秒级定时器
time_s<26
2s 误差-1ms
*/
void set_timer1_s(timercallback userFunc, double time_s)
{
	timer1_isr_init();
	//timer1_enable(1, TIM_EDGE, TIM_LOOP);//分频，是否优先，是否重填
	//timer1_write(8000000);//count count<8388607
	//timer1 time = (16^分频)*count*0.0000000125  单位：s

	/*
	//2s 误差-1ms
	timer1_enable(2,TIM_EDGE,TIM_LOOP);
	timer1_write(312500*2);
	*/
	timer1_enable(2, TIM_EDGE, TIM_LOOP);
	timer1_write((uint32)(312500 * time_s));
	timer1_enabled();
	timer1_attachInterrupt(userFunc);
}

/*
毫秒定时器
time_ms < 1,677
*/
void set_timer1_ms(timercallback userFunc, uint32_t time_ms)
{
	timer1_isr_init();

	/*
	//timer1_enable(1, TIM_EDGE, TIM_LOOP);//分频，是否优先，是否重填
	//timer1_write(8000000);//count count<8388607
	//timer1 time = (16^分频)*count*0.0000000125  单位：s  //默认esp8266时钟频率80MHz
	//1ms 误差~1ms ??? 定时100ms误差<1ms,定时10ms误差也是1ms，我也不知道为啥，
	//我没有示波器给我测试，但是我算出来的数据填充之后串口输出时间差就是这样，可能是串口耽误了时间吧
	timer1_enable(2,TIM_EDGE,TIM_LOOP);
	timer1_write(312500*2);
	*/
	timer1_enable(1, TIM_EDGE, TIM_LOOP);
	timer1_write((uint32)(5000 * time_ms));
	timer1_enabled();
	timer1_attachInterrupt(userFunc);
}

short get_wifi()
{
	Serial.print("Connecting to ");
	Serial.println(WIFI_ssid);
	//WiFi.mode(WIFI_RESUME);
	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_ssid, WIFI_password);
	//WiFi.smartConfigDone();//智能配网？
	short i = 20;

	while ((WiFi.status() != WL_CONNECTED) && i-- > 0)
	{
		delay(500);
		Serial.print(".");

		//长按 10*500 ms
		//删除之前记住的WiFi账号和密码，然后重新启动系统
		if (digitalRead(anjian1) == LOW)
		{
			//亮灯指示一下
			//pinMode(LED_BUILTIN, OUTPUT);
			digitalWrite(LED_BUILTIN, LOW);
			short count_anjian = 0; //对按键按下的时间计数，超过5s就清除wifidata.txt文件，然后重新启动系统
			while (digitalRead(anjian1) == LOW && count_anjian < 10)
			{
				Serial.printf("-%dS -", 10 - count_anjian);
				count_anjian = count_anjian + 1;
				delay(500);
			}
			if (count_anjian >= 10)
			{
				//删除之前记住的WiFi账号和密码，然后重新启动系统
				Serial.println("delete & restart");
				file_delete_wifidata();
				resetFunc();
			}
			digitalWrite(LED_BUILTIN, HIGH);
		}
	}
	if (WiFi.status() == WL_CONNECTED)
	{
		Serial.println("WiFi connected");
		Serial.println("IP address: ");
		Serial.println(WiFi.localIP());
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
将数据放在一个数组里发送。
*/
int set_databack()
{
	int i, k, count_char;
	tcp_send_data[0] = '#'; //在这里插入开始符号
	count_char = 1;
	for (i = 0; i < MAX_NAME; i++)
	{
		k = 0;
		while (str_data_names[i][k] != 0) //把数据的名字填充到数组里
		{
			tcp_send_data[count_char] = str_data_names[i][k];
			k++;
			count_char++;
		}
		tcp_send_data[count_char++] = ':'; //在这里插入分隔符
		//char** str_data_names = { "温度" ,"湿度","灯0" ,"灯1" };
		switch (i) //把数据填充到数组里
		{
		case 0:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%.1f°C", dht11_data.temperature);
			break;
		case 1:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%.1f%%", dht11_data.humidity);
			break;
		case 2:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d%%", system_adc_read() * 100 / 1024);
			break;
		case 3:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", LED1);
			break;
		case 4:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", switch_1);
			break;
		case 5:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", LED2);
			break;
		case 6:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", switch_2);
			break;
		case 7:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%dS", switch_2_light_up_TIME_s);
			break;
		case 8:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%dS", switch_2_light_up_time_x_s);
			break;
		case 9:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", TEMPERATURE_ERROR_HIGH);
			break;
		case 10:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", TEMPERATURE_ERROR_LOW);
			break;
		case 11:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", light_qu_yu);
			break;
		case 12:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", power_save);
			break;
		}
		tcp_send_data[count_char++] = '#'; //在这里插入单个数据结束符
	}
	//Serial.printf("  count_char :%d  ", count_char);
	return count_char;
}

/*
保存各项配置的状态，开机可以重新恢复状态

//断电记忆不储存断电记忆的状态，
*/
short file_save_stut()
{
	if (LittleFS.begin())
	{ // 启动SPIFFS
		Serial.println("SPIFFS Started.");
	}
	else
	{
		Serial.println("SPIFFS Failed to Start.");
		return -2; //文件系统启动失败
	}

	//因为保存的状态需要经常更新？
	//将保存配置设为可选选项？

	/*
uint8_t switch_1 = 1;
uint8_t switch_2 = 1;
uint8_t LED1 = 0;
uint8_t LED2 = 0;
short switch_2_light_up_TIME_s = 3;//重新加载的值
short TEMPERATURE_ERROR_HIGH = 40;
short TEMPERATURE_ERROR_LOW = 10;
uint8_t light_qu_yu = 5;//补光区间
	//1 1
	//2 1
	//3 1
	//4 1
	//5 2
	//6 2
	//7 2
	//8 1
*/
	/*
	规定：
		按照上一个注释的顺序按字节写入内容
	*/
	int cou = 0;
	File dataFile = LittleFS.open(stut_data_file, "w"); // 建立File对象用于向SPIFFS中的file对象（即/notes.txt）写入信息
	cou = cou + dataFile.write(switch_1);
	cou = cou + dataFile.write(switch_2);
	cou = cou + dataFile.write(LED1);
	cou = cou + dataFile.write(LED2);
	cou = cou + dataFile.write((uint8_t)(switch_2_light_up_TIME_s >> 8));
	cou = cou + dataFile.write((uint8_t)(switch_2_light_up_TIME_s & 255));
	cou = cou + dataFile.write((uint8_t)(TEMPERATURE_ERROR_HIGH >> 8));
	cou = cou + dataFile.write((uint8_t)(TEMPERATURE_ERROR_HIGH & 255));
	cou = cou + dataFile.write((uint8_t)(TEMPERATURE_ERROR_LOW >> 8));
	cou = cou + dataFile.write((uint8_t)(TEMPERATURE_ERROR_LOW & 255));
	cou = cou + dataFile.write(light_qu_yu);
	//dataFile.flush();
	dataFile.close(); // 完成文件写入后关闭文件
	Serial.printf("write count = %d ", cou);
	return 1;
}

/*
读取各项配置的状态，开机可以重新恢复状态

//断电记忆不储存断电记忆的状态，
*/
short file_read_stut()
{
	if (LittleFS.begin())
	{ // 启动SPIFFS
		Serial.println("SPIFFS Started.");
	}
	else
	{
		Serial.println("SPIFFS Failed to Start.");
		return -2; //文件系统启动失败
	}

	if (LittleFS.exists(stut_data_file))
	{
		Serial.print(stut_data_file);
		Serial.println(" FOUND.");
	}
	else
	{
		Serial.print(stut_data_file);
		Serial.print(" NOT FOUND.");
		get_files_name();
		return -1; //未找到WiFi文件
	}
	File dataFile = LittleFS.open(stut_data_file, "r"); // 建立File对象用于向SPIFFS中的file对象（即/notes.txt）写入信息
	//1 1
	//2 1
	//3 1
	//4 1
	//5 2
	//6 2
	//7 2
	//8 1
	Serial.printf("dataFile.size() %d", dataFile.size());
	if (dataFile.size() < 11) //我存了11字节的数据，多了一字节是结束符号吧
	{
		Serial.print(stut_data_file);
		Serial.print(" DATA ERROR.");
		LittleFS.remove(stut_data_file);
		return -3; //数据内容错误
	}
	//Serial.printf(" switch_1 %d ", (uint8_t)dataFile.read());
	//Serial.printf(" switch_2 %d ", (uint8_t)dataFile.read());
	//Serial.printf(" LED1 %d ", (uint8_t)dataFile.read());
	//Serial.printf(" LED2 %d", (uint8_t)dataFile.read());
	//Serial.printf(" switch_2_light_up_TIME_s %d ", (short)((uint8_t)dataFile.read() << 8 | (uint8_t)dataFile.read()));
	//Serial.printf(" TEMPERATURE_ERROR_HIGH %d ", (short)((uint8_t)dataFile.read() << 8 | (uint8_t)dataFile.read()));
	//Serial.printf(" TEMPERATURE_ERROR_LOW %d ", (short)((uint8_t)dataFile.read() << 8 | (uint8_t)dataFile.read()));
	//Serial.printf(" light_qu_yu %d ", (uint8_t)dataFile.read());
	switch_1 = (uint8_t)dataFile.read();
	switch_2 = (uint8_t)dataFile.read();
	LED1 = (uint8_t)dataFile.read();
	LED2 = (uint8_t)dataFile.read();
	switch_2_light_up_TIME_s = (short)((uint8_t)dataFile.read() << 8 | (uint8_t)dataFile.read());
	TEMPERATURE_ERROR_HIGH = (short)((uint8_t)dataFile.read() << 8 | (uint8_t)dataFile.read());
	TEMPERATURE_ERROR_LOW = (short)((uint8_t)dataFile.read() << 8 | (uint8_t)dataFile.read());
	light_qu_yu = (uint8_t)dataFile.read();
	dataFile.close(); // 完成文件写入后关闭文件
	//Serial.printf("TEMPERATURE_ERROR_HIGH %d", TEMPERATURE_ERROR_HIGH);
	//Serial.printf("TEMPERATURE_ERROR_LOW %d", TEMPERATURE_ERROR_LOW);
	return 1;
}

/*
将打包好的数据，用TCP发送出去
*/
char back_send_tcp(WiFiClient client)
{
	if (client.connected()) //函数第一次执行loop循环的时候这里可能会出错，因为 client 第一次赋值为局部变量，在setuo 中修改他的初始化就可以了
	{
		//在这里合成需要发送出去的传感器数据？
		client.write(tcp_send_data, set_databack());
		//client.flush();
		return 1;
	}
	else
	{
		//结束此次 loop ，到开始位置，重新连接TCP
		client.stop();
		return -1;
	}
}

char tcp_server_get_wifi_data()
{
	//WiFi.mode()
	char data[1024];
	int ind = 0;
	WiFi.mode(WIFI_RESUME);
	IPAddress softLocal(192, 168, 128, 1); // 1 设置内网WIFI IP地址
	IPAddress softGateway(192, 168, 128, 1);
	IPAddress softSubnet(255, 255, 255, 0);
	WiFi.mode(WIFI_AP);
	WiFi.softAPConfig(softLocal, softGateway, softSubnet);
	WiFi.softAP("HCC_APP", "12345678");
	Serial.print("Connected to ");
	Serial.println("esp8266");
	Serial.print("IP Address: ");
	Serial.println(WiFi.localIP()); //串口监视器显示IP地址
	// Start a TCP Server on port 5045
	WiFiServer server(9997); //端口5045，自定义（避免公用端口）
	WiFiClient client;
	// Start the TCP server
	server.begin();
	while (1)
	{
		if (!client.connected())
		{
			//try to connect to a new client
			client = server.available();
		}
		else
		{
			if (client.available() > 0)
			{
				//Serial.println("Connected to client");

				while (client.available())
				{
					data[ind] = client.read(); //读取client端发送的字符
					ind++;
				}
				client.flush();
				//处理其他设备发送过来的数据
				//找到"wifi:"字符
				if (str1_find_str2_(data, ind, "+SSID:") >= 0)
				{
					for (int i = 6, k = 0; i < ind && i < WIFI_ssid_len; i++, k++)
					{
						WIFI_ssid[k] = data[i];
						data[i] = 0; //转移了的数据清零
						if (WIFI_ssid[k] == '\n' || WIFI_ssid[k] == '\0')
						{
							WIFI_ssid[k] = '\0';
							break;
						}
					}
					client.print(WIFI_ssid); //在client端回复
				}
				else if (str1_find_str2_(data, ind, "+PW:") >= 0)
				{
					for (int i = 4, k = 0; i < ind && i < WIFI_password_len; i++, k++)
					{
						WIFI_password[k] = data[i];
						data[i] = 0; //转移了的数据清零
						if (WIFI_password[k] == '\n' || WIFI_password[k] == '\0')
						{
							WIFI_password[k] = '\0';
							break;
						}
					}
					client.print(WIFI_password); //在client端回复
				}
				else if (str1_find_str2_(data, ind, "+UID:") >= 0)
				{
					//len("+UID:")=4
					UID = str_to_u64(data + 4, ind - 4);
					data[0] = 0;				  //转移了的数据清零
					client.printf("UID:%d", UID); //在client端回复
				}
				else
				{
					continue;
				}
				for (int j = 0; j < ind; j++)
				{
					Serial.print(data[j]);
				}
				Serial.print("#WIFI_ssid:");
				Serial.print(WIFI_ssid);
				Serial.print("#WIFI_password:");
				Serial.print(WIFI_password);
				Serial.print('#');
				Serial.print("\n");
				if (WIFI_ssid[0] != 0 && WIFI_password[0] != 0)
				{
					delay(5);										   //延时五毫秒，不然最后一个数据可能发不出去
					client.printf(",uid=%d,chip_id=%d", UID, CHIP_ID); //在client端回复
					delay(5);										   //延时五毫秒，不然最后一个数据可能发不出去
					file_save_wifidata();
					return 1;
				}
				ind = 0;
			}
		}
	}
}

void set_data_(short i, short value)
{
	switch (i)
	{ //在这里修改控件的状态
	case 3:
		if (value > -1 && value < 2)
		{
			LED1 = value;
		}
		break;

	case 4:
		if (value > -1 && value < 4)
		{
			switch_1 = value;
		}
		break;

	case 5:
		if (value > -1 && value < 2)
		{
			LED2 = value;
		}
		break;

	case 6:
		if (value > -1 && value < 4)
		{
			switch_2 = value;
		}
		break;

	case 7:
		if (value > 0 && value < 301)
		{
			switch_2_light_up_TIME_s = value;
		}
		break;
	case 9:
		if (value >= 0 && value <= 40)
		{
			TEMPERATURE_ERROR_HIGH = value;
		}
		break;
	case 10:
		if (value >= 0 && value <= 40)
		{
			TEMPERATURE_ERROR_LOW = value;
		}
		break;
	case 11:
		if (value >= 0 && value <= 10)
		{
			light_qu_yu = value;
		}
		break;
	case 12:
		if (value == 0 || value == 1)
		{
			power_save = value;
		}
		break;
	}
}

void setup()
{
	//LittleFS.format();//第一次使用flash需要将flash格式化
	Serial.begin(115200);
	//CHIP_ID = ESP.getFlashChipId();
	CHIP_ID = ESP.getChipId();
	Serial.printf("getFlashChipId %d \n", ESP.getFlashChipId()); //这个id是假的，不知道为啥，两个esp的一样
	Serial.printf("getChipId %d  \n", ESP.getChipId());
	//定时器初始化
	//ticker_DHT11.attach(2, timer2_worker);//.detach();//调用这个可以关闭
	//读取FLASH芯片的唯一ID，
	//Serial.printf("Flash real id（唯一标识符）:   %08X\n", ESP.getFlashChipId());

	pinMode(light, INPUT);	  //光
	pinMode(anjian1, INPUT);  //按键1
	pinMode(shengyin, INPUT); //d2 声音
	pinMode(LED_BUILTIN, OUTPUT);

	short stat = file_read_wifidata();
	Serial.printf("star%d", stat);
	if (stat == -1)
	{

		digitalWrite(LED_BUILTIN, LOW);
		tcp_server_get_wifi_data();
		digitalWrite(LED_BUILTIN, HIGH);
	}
	else if (stat == -2)
	{
		Serial.print("flash error ,file open error -2");
		ESP.deepSleep(300 * 1000); //us
		resetFunc();
	}
	//连接wifi
	if (WIFI_password[0] == '\0' || WIFI_ssid == '\0')
	{
		Serial.println("delete & restart");
		file_delete_wifidata();
		//resetFunc();
	}
	Serial.printf("#WIFI_ssid:%s\nWIFI_password:%s\nUID:%ld", WIFI_ssid, WIFI_password, UID);

	Serial.printf("CHIP_ID %x 111111  \n", CHIP_ID);
	if (get_wifi() == 0)
	{
		ESP.deepSleep(20000000, WAKE_RFCAL);
	}
	Serial.printf(" file_read_stut %d ", file_read_stut());
	DHT11_init(5); //这个是DHT11.h/DHT11.c里的函数
				   //ESP.deepSleep(20000000, WAKE_RFCAL);
				   //开始循环之前先调用一次，初始化一下温湿度的值
}

void loop()
{
	static short error_wifi_count = 0;
	int beeeeee = 0;
	timer1_disable();

	if (WiFi.status() != WL_CONNECTED)
	{
		//计次，超过三次就复位

		if (get_wifi() == 0)
		{
			if (error_wifi_count > 1 && error_wifi_count < 2)
			{
				WiFi.mode(WIFI_RESUME); //重新连接wifi
			}
			else if (error_wifi_count == 3)
			{
				//超过6次链接失败，复位程序，重启
				Serial.print("\r\ndeepSleep\r\n");
				ESP.deepSleep(20000000, WAKE_RFCAL);
				//Serial.print("\r\nresetFunc\r\n");
				//resetFunc();
			}
			//system_deep_sleep();
			error_wifi_count++;
			return;
		}
		//set_timer1_s(realy_DHT11, 2);
	}
	WiFiClient client;
	short stat;
	if (client.connect(MYHOST, TCP_PORT))
	{
		//client.printf("hello from ESP8266 %d", ESP.getFlashChipId());
		if (UID != 0)
		{
			client.printf("+UID:%d", UID);
		}

		String str1 = "class_id=1,chip_id=" + String(CHIP_ID) + ",ip=" + WiFi.localIP().toString() + ",mac=" + WiFi.macAddress();
		client.print(str1);
		error_tcp_sum = 0;
	}
	else
	{
		Serial.printf("tcp error,loop end :%d  \n", error_tcp_sum++);
		delay(3000);
		if (error_tcp_sum > 3 && error_tcp_sum < 6)
		{
			WiFi.mode(WIFI_RESUME); //重新连接wifi
		}
		else if (error_tcp_sum == 6)
		{
			//超过三次链接失败，复位程序，重启
			resetFunc();
		}

		return;
	}
	Serial.print("tcp ok");

	stat = timeout_back(client, 3000);
	if (stat == 1)
	{
		//get_tcp_data();
		//这里还要改改，这里只是把信息接受之后直接输出了，并没有处理
		//这里收到的信息可能是服务器返回的第一条信息
		Serial.println(tcp_data + get_tcp_data(client));
		//beeeeee 临时储存一下+EID的开始位置
		beeeeee = str1_find_str2_(tcp_data, tcp_data_len, "+EID");
		if (beeeeee >= 0)
		{
			EID = str_to_u64(tcp_data + beeeeee, tcp_data_len);
			UDP_head_data = "+EID=" + String(EID) + ",chip_id=" + String(CHIP_ID) + ",";
			Serial.print(UDP_head_data);
		}
		tcp_data_len = 0;
	}
	else if (stat == 0)
	{

		Serial.print("servier error ");
		ESP.deepSleep(20000000, WAKE_RFCAL);
		return;
	}

	unsigned long time_old_ms = millis();
	unsigned long beeeee_time_old_ms = millis();
	short len_old;
	pinMode(jd2, OUTPUT); //连个
	pinMode(jd1, OUTPUT);
	set_timer1_s(realy_DHT11, 2);
	while (1) //tcp断开之后无法重新链接，我只能重新声明试试，但是好像也没什么用处，只能计次，然后软件复位程序
	{
		//time_old_ms = millis();
		//隔一段时间就发送一次本机数据，怕tcp失效
		if (millis() - time_old_ms > 5000)
		{
			//TCP发送一些数据
			if (back_send_tcp(client) == -1)
				return;
			time_old_ms = millis();
		}

		//Serial.print(system_adc_read());//读取A0ADC的值，最大1024 我猜的略略略

		if (millis() - beeeee_time_old_ms > 10)
		{
			//进行操作
			if (beeeeee > 10) //要大于五是因为偶尔会采样出错，一般是连续的三个，正常人发出的声音远大于此
			{
				//修改开灯寄存器
				switch_2_light_up_time_x_s = switch_2_light_up_TIME_s;
				//Serial.printf("switch_2_light_up_time_x_s  :%d \r\n", switch_2_light_up_time_x_s);
			}

			//Serial.printf(" %d", beeeeee);

			beeeee_time_old_ms = millis(); //更新时间
			beeeeee = 0;				   //更新计数器

			if (!UDP_send_data.equals(""))
			{
				UDP_Send(MYHOST, UDP_PORT, UDP_send_data);
				UDP_send_data = "";
			}
		}

		//micros();
		//int time_old = micros();//这个是我用来测试TCP响应时间的，读取当前时间，之后用新时间减去这个值
		//如果回复重要，就多等一下，把 timeout_ms_max 改大一点
		stat = timeout_back_us(client, 100); //等待100us tcp是否有数据返回
		//对声音采样
		//直接加上就可以了，反正就是010101001010101
		beeeeee = beeeeee + digitalRead(shengyin);
		//有收到TCP数据
		if (stat == 1)
		{
			//Serial.printf("回复响应时间：%d \n", micros() - time_old);
			len_old = get_tcp_data(client);
			//如果需要对TCP链接返回的数据进行处理，请在这后面写，-----------------------------------------------------------------------
			//示例
			//将TCP返回的数据当作字符串输出
			Serial.print(tcp_data + len_old);
			//Serial.printf("udp send id=%d\n", debug_udp_count);

			switch (*(tcp_data + len_old))
			{
			case '+':
			case 'G':
			case 'g':
				if (back_send_tcp(client) == -1)
					return;
				time_old_ms = millis();
				break; // 跳出 switch
			case '@':  //set
				while (len_old >= 0 && len_old < tcp_data_len)
				{
					for (short i = 0; i < MAX_NAME; i++)
					{
						if (0 <= str1_find_str2_1(tcp_data, len_old, str1_find_char_1(tcp_data, len_old, tcp_data_len, ':'), str_data_names[i]))
						{
							short value = str_to_u16(tcp_data + str1_find_char_1(tcp_data, len_old, tcp_data_len, ':')); //将赋值的分隔符之后的数据转换成u16
							//这里作为调试用，串口发送很占时间
							Serial.printf("	set id = %d	value= %d \r\n", i, value);
							set_data_(i, value);
							break;
						}
					}

					tcp_data[len_old] = 0; //清楚标志位的数据
					len_old = str1_find_char_1(tcp_data, len_old + 1, tcp_data_len, '@');
					//只识别 @ 类型的数据，get类型的数据一般不会组合发送，舍弃此部分
				}
				//所有的指令已经执行完毕
				brightness_work(); //更新一下光控灯的状态
				//TCP 打包返还自己的状态
				if (back_send_tcp(client) == -1)
					return;
				if (power_save == 1) //实时更新断电记忆的东西
				{
					file_save_stut();
					power_save = 0;
				}
				break; //跳出 switch
			}
			tcp_data[0] = 0;
			tcp_data_len = 0; //因为我没有在其他位置调用数据接收函数，所以我处理完之后全部清除了
		}
		//TCP链接失效
		else if (stat == 0)
		{
			Serial.printf("tcp error,loop end :%d", error_tcp_sum++);
			delay(1000);
			return;
		}
	}
}
