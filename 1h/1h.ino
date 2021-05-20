#include <ESP8266WiFi.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LittleFS.h>
#include "test.h"
#include "savevalues.h"
#include "mywifi.h"
#include "mytcp.h"
extern "C"
{
#include "myconstant.h"
#include "DHT11.h"
#include "mystr.h"
}

char WIFI_ssid[WIFI_SSID_LEN] = {'\0'};
char WIFI_password[WIFI_PASSWORD_LEN] = {'\0'};
u64 UID = 0;
u64 EID = 0;
char str_EID[22] = {0};
uint32_t CHIP_ID = 0;

struct Tcp_cache my_tcp_cache; //TCP缓存数组

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
const char *str_data_names[MAX_NAME] = {"温度",
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
										"@保存当前为断电记忆[0-2]"};

const char *MODE_INFO = "@开关1模式[0-3]:手动，声控，光控，光声混控@开关2模式[0-3]:手动，声控，光控，光声混控@断电记忆[0-2]:本次不，仅本次，所有";
struct DHT11_data dht11_data = {666, 666};

const char *wifi_ssid_pw_file = "/wifidata.txt"; //储存 WiFi 账号和密码的文件
const char *stut_data_file = "/stutdata.txt";	 //储存设备各功能配置状态的文件
const char *MYHOST = "121.89.243.207";			 //服务器 ip 地址
const uint16_t TCP_PORT = 9999;
const uint16_t UDP_PORT = 9998;

//两个开关，当他为2时，是自动模式，其他时候读取12 和14号脚的电平
uint8_t LED1 = 0;
uint8_t switch_1 = 2;
uint8_t LED2 = 0;
uint8_t switch_2 = 2;
short switch_light_up_TIME_s = 30;	//重新加载的值//声控灯开启时长
short switch_light_up_time_x_s = 0; //计数器用
short TEMPERATURE_ERROR_HIGH = 40;
short TEMPERATURE_ERROR_LOW = 10;
uint8_t light_qu_yu = 5; //补光区间
uint8_t power_save = 0;	 //断电记忆

//下面定义几个引脚的功能
const uint8_t jd1 = 14;		//1号继电器
const uint8_t jd2 = 12;		//2号继电器
const uint8_t light = 13;	//光敏逻辑输入
const uint8_t shengyin = 4; //声音逻辑输入
const uint8_t anjian1 = 0;	//按键1输入
const uint8_t dht11 = 5;	//按键1输入

//其他函数声明
void timer1_worker();
void read_dht11();
void set_timer1_s(timercallback userFunc, double time_s);
void set_timer1_ms(timercallback userFunc, uint32_t time_ms);

/*
UDP发送函数封装起来，方便调用
调用示例 ：UDP_Send(MYHOST, UDP_PORT, "UDP send 汉字测试 !");
return :
			-1	无法链接
			0	发送失败
			1	发送成功
*/
short UDP_Send(const char *UDP_IP, uint16_t UDP_port, String udp_send_data)
{
	if (WiFi.status() != WL_CONNECTED)
	{
		return -1;
	}
	WiFiUDP Udp;
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
	digitalWrite(pin_x, *LEDx);
}

/*根据模式更新继电器开关的状态*/
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
	short t = dht11_read_data(&dht11_data);
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
			UDP_send_data = UDP_send_data + UDP_head_data + "temperature high";
			//UDP_Send(MYHOST, UDP_PORT, UDP_head_data + "temperature high");
		}
		else if (dht11_data.temperature < TEMPERATURE_ERROR_LOW)
		{
			UDP_send_data = UDP_send_data + UDP_head_data + "temperature low";
			//UDP_Send(MYHOST, UDP_PORT, UDP_head_data + "temperature low");
		}
	}
}

/*每隔 DHT11_SPACE_OF_TIME_ms 读取DHT11*/
void dht11_get()
{
	static short timer2_count = TIMER2_COUNT; //
	timer2_count++;
	if (timer2_count >= TIMER2_COUNT)
	{
		//先拉低，LOW_PIN_ms 之后调用读取函数
		set_timer1_ms(read_dht11, (unsigned int)dht11_read_ready());
		timer2_count = 0;
		return;
	}
}

/*
定时器工作内容
*/
void timer1_worker()
{
	//delay(20);//时间中断函数里不可以用delay
	clear_wifi_data(wifi_ssid_pw_file); //长按按键1清除wifi账号密码记录
	brightness_work();					//更新继电器状态
	shengyin_timeout_out();				//更新声控灯倒计时
	dht11_get();						//调用DHT11的读取函数
}

/*
此函数在定时中断中调用，处理温湿度传感器通讯协议中18ms下拉
*/
void read_dht11()
{
	//让DHT11的信号引脚拉低，等待20ms，之后调用get_DHT11_DATA() 开始正式调用读取函数
	set_timer1_ms(timer1_worker, TIMER1_timeout_ms - LOW_PIN_ms); //正常时间之后恢复 timer1_worker 的工作
	DHT11_read_and_send();
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

/*毫秒定时器 定时中断函数里禁止调用 delay 进行延时操作，调用必暴毙
time_ms < 1,677
userFunc 需要定时调用执行的函数名*/
void set_timer1_ms(timercallback userFunc, uint32_t time_ms)
{
	timer1_isr_init(); //系统函数，初始化定时器

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
	timer1_enabled();				  //使能中断
	timer1_attachInterrupt(userFunc); //填充
}

/*将数据放在一个数组里发送。 返回数据的长度*/
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
			count_char = count_char + sprintf(tcp_send_data + count_char, "%dS", switch_light_up_TIME_s);
			break;
		case 8:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%dS", switch_light_up_time_x_s);
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

/*在这里修改控件的状态 i是名称对应的数组索引，value是用户赋值*/
void set_data_(short i, short value)
{
	switch (i)
	{ //在这里修改控件的状态
	case 3:
		if (value > -1 && value < 2)
		{
			LED1 = value; //1号继电器
		}
		break;

	case 4:
		if (value > -1 && value < 4)
		{
			switch_1 = value; //1号继电器 工作模式
		}
		break;

	case 5:
		if (value > -1 && value < 2)
		{
			LED2 = value; //2号继电器
		}
		break;

	case 6:
		if (value > -1 && value < 4)
		{
			switch_2 = value; //2号继电器 工作模式
		}
		break;

	case 7:
		if (value > 0 && value < 301)
		{
			switch_light_up_TIME_s = value;
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
		if (value >= 0 && value <= 2)
		{
			power_save = value;
		}
		break;
	}
}

/*添加需要保存的变量，上限为 list_values_len_max */
void add_values()
{
	add_value(&switch_1, sizeof(switch_1));
	add_value(&switch_2, sizeof(switch_2));
	add_value(&LED1, sizeof(LED1));
	add_value(&LED2, sizeof(LED2));
	add_value(&switch_light_up_TIME_s, sizeof(switch_light_up_TIME_s));
	add_value(&TEMPERATURE_ERROR_HIGH, sizeof(TEMPERATURE_ERROR_HIGH));
	add_value(&TEMPERATURE_ERROR_LOW, sizeof(TEMPERATURE_ERROR_LOW));
	add_value(&light_qu_yu, sizeof(light_qu_yu));
}

void setup()
{
	pinMode(16, OUTPUT);
	digitalWrite(16, HIGH); //不知道为啥，这个模块的初始状态是LOW，然后我一插上跳线帽就开始无限重启//是电压低的问题，默认未初始化的电压低于判定电压，在 nodemcu 上没有出现错误可能是因为产品型号/批次的不同，经过电压表测量，nodemcu的电压在0.8V左右，单个小模块的电压不到0.3
	add_values();			//挂载读取信息。//这里可以优化，仅在读取写入的时候使用数组，建立//但是也没多大用，一个不超过50字节的数组
	set_anjian1(anjian1);	//配置wifi的清除数据按键

	//LittleFS.format();//第一次使用flash需要将flash格式化

	Serial.begin(115200);
	//CHIP_ID = ESP.getFlashChipId();
	//Serial.printf("unsigned long %d \n", sizeof(unsigned long)); //这个id是假的，不知道为啥，两个esp的一样
	//Serial.printf("long long %d  \n", sizeof(long long));
	CHIP_ID = ESP.getChipId();
	Serial.printf("getFlashChipId %d \n", ESP.getFlashChipId()); //这个id是假的，不知道为啥，两个esp的一样
	Serial.printf("getChipId %d  \n", ESP.getChipId());

	pinMode(light, INPUT);	  //光
	pinMode(anjian1, INPUT);  //按键1
	pinMode(shengyin, INPUT); //d2 声音
	pinMode(LED_BUILTIN, OUTPUT);

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
		Serial.print("flash error ,file open error -2,deepSleep");
		ESP.deepSleep(300 * 1000); //us
	}
	//连接wifi
	if (WIFI_password[0] == '\0' || WIFI_ssid == '\0')
	{
		Serial.println("delete & restart");
		file_delete(wifi_ssid_pw_file);
	}
	Serial.printf("#WIFI_ssid:%s  WIFI_password:%s  UID:%ld ", WIFI_ssid, WIFI_password, UID);

	Serial.printf("CHIP_ID %x \n", CHIP_ID);
	if (get_wifi(WIFI_ssid, WIFI_password, wifi_ssid_pw_file) == 0)
	{
		Serial.print("flash error ,file open error -2,deepSleep");
		ESP.deepSleep(20000000, WAKE_RFCAL);
	}

	//Serial.printf(" file_read_stut %d ", file_read_stut());
	Serial.printf(" read_values %d \n", read_values(stut_data_file));
	dht11_init(dht11); //这个是DHT11.h/DHT11.c里的函数，初始化引脚
}

void loop()
{
	static short error_wifi_count = 0;
	static int error_tcp_sum = 0; //tcp链接失败计次
	int beeeeee = 0;
	timer1_disable();

	if (WiFi.status() != WL_CONNECTED)
	{
		//计次，超过三次就休眠
		if (get_wifi(WIFI_ssid, WIFI_password, wifi_ssid_pw_file) == 0)
		{
			Serial.printf("error_wifi_count==%d\r\n", error_wifi_count);
			if (error_wifi_count == 2)
			{
				WiFi.mode(WIFI_OFF); //断开wifi 之后重新连接wifi
				delay(1000);
			}
			else if (error_wifi_count == 3)
			{
				//超过6次链接失败，复位程序，重启
				Serial.print("\r\ndeepSleep 20S\r\n");
				ESP.deepSleep(20000000, WAKE_RFCAL);
			}
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
			client.printf("+UID:%llu", UID);
		}

		String str1 = "class_id=1,chip_id=" + String(CHIP_ID) + ",ip=" + WiFi.localIP().toString() + ",mac=" + WiFi.macAddress();
		client.print(str1);
		error_tcp_sum = 0;
	}
	else
	{
		Serial.printf(" 1 error_tcp_sum=%d \r\n", error_tcp_sum++);
		delay(3000); //等待3S再重连
		if (error_tcp_sum > 3 && error_tcp_sum < 6)
		{
			Serial.print("WiFi.mode(WIFI_OFF);\r\n");
			WiFi.mode(WIFI_OFF); //重新连接wifi
		}
		else if (error_tcp_sum == 6)
		{
			//超过三次链接失败，复位程序，重启
			Serial.print("\r\nresetFunc\r\n");
			ESP.restart(); //resetFunc();
		}
		return;
	}
	Serial.print("tcp ok");

	stat = timeout_back_ms(client, 3000);
	if (stat == 1)
	{
		//这里收到的信息可能是服务器返回的第一条信息
		Serial.println(my_tcp_cache.data + get_tcp_data(client, &my_tcp_cache));
		//beeeeee 临时储存一下+EID的开始位置
		beeeeee = str1_find_str2_(my_tcp_cache.data, my_tcp_cache.len, "+EID");
		if (beeeeee >= 0)
		{
			EID = str_to_u64(my_tcp_cache.data + beeeeee, my_tcp_cache.len, &stat);
			if (stat != 1)
			{
				//值转换出错，溢出或未找到有效值
				Serial.print("\r\nnot found eid,deepSleep\r\n");
				ESP.deepSleep(20000000, WAKE_RFCAL);
			}
			sprintf(str_EID, "%llu", EID); //垃圾string(),居然不能装换long long类型的数据，还要我自己动手
			UDP_head_data = "+EID=" + String(str_EID) + ",chip_id=" + String(CHIP_ID) + ",";
			Serial.print(UDP_head_data);
		}
		my_tcp_cache.len = 0;
	}
	else if (stat == 0)
	{
		Serial.print("\r\nservier error,deepSleep\r\n");
		ESP.deepSleep(20000000, WAKE_RFCAL);
		//return;
	}

	unsigned long time_old_ms = millis();
	unsigned long beeeee_time_old_ms = millis();
	short len_old;
	brightness_work(); //初始化引脚之前，先调整高低电平，减少不必要的继电器响声
	pinMode(jd2, OUTPUT);
	pinMode(jd1, OUTPUT);
	set_timer1_ms(timer1_worker, TIMER1_timeout_ms); //强制重新初始化定时中断，如果单纯的使用 dht11_get 里的过程初始化，有概率初始化失败
	//（仅在程序复位的时候可以成功，原因：timer2_count 没有复位就不会被初始化，自然调用不到定时器的初始化函数），
	dht11_get(); //读取dht11的数据，顺便启动定时器//这里有问题，当断网重连之后，定时器函数有可能不会被重新填充
	while (client.connected())	 //tcp断开之后无法重新链接，我只能重新声明试试，但是好像也没什么用处???，只能计次，然后软件复位程序
	{
		//time_old_ms = millis();
		//隔一段时间就发送一次本机数据，怕tcp失效
		if (millis() - time_old_ms > HEART_BEAT_ms)
		{
			Serial.print('#');
			//TCP发送一些数据
			if (back_send_tcp_(client, tcp_send_data, set_databack()) == -1)
			{
				Serial.printf(" 4 error_tcp_sum=%d \r\n", error_tcp_sum++);
				return;
			}
			//在这里插入心跳包的返回值检测，超时未回复，就认定链接失效，重新建立tcp的链接
			/*实现方式：添加两个标志？记录发送状态和接收状态，判断发送和接受状态的情况*/
			time_old_ms = millis();
		}

		if (millis() - beeeee_time_old_ms > 10)
		{
			//进行操作
			if (beeeeee > 10) //要大于五是因为偶尔会采样出错，一般是连续的三个，正常人发出的声音远大于此
			{
				//更新声控灯剩余时长
				switch_light_up_time_x_s = switch_light_up_TIME_s;
				//Serial.printf("switch_light_up_time_x_s  :%d \r\n", switch_light_up_time_x_s);
			}

			//Serial.printf(" %d", beeeeee);
			beeeee_time_old_ms = millis(); //更新时间
			beeeeee = 0;				   //更新计数器

			if (UDP_send_data == NULL)
			{
				UDP_send_data = "";
			}
			else if (!UDP_send_data.equals(""))
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
			len_old = my_tcp_cache.len;
			stat = get_tcp_data(client, &my_tcp_cache);
			//这里还有问题， get_tcp_data 可能返回-1 这是溢出的标志
			/*
				if(stat==-1)
				{
					//这里填写处理溢出的方案
				}
			*/
			//如果需要对TCP链接返回的数据进行处理，请在这后面写，-----------------------------------------------------------------------
			//示例
			//将TCP返回的数据当作字符串输出
			Serial.print(my_tcp_cache.data + len_old);
			//Serial.printf("udp send id=%d\n", debug_udp_count);

			switch (*(my_tcp_cache.data + len_old))
			{
			case '+': //获取传感器和模式的信息
			case 'G':
			case 'g':
				if (back_send_tcp_(client, tcp_send_data, set_databack()) == -1)
					return;
				time_old_ms = millis();
				break; // 跳出 switch
			case 'I':  //获取一些模式id的详细描述
			case 'i':
				if (back_send_tcp(client, MODE_INFO) == -1)
					return;
				time_old_ms = millis();
				break; // 跳出 switch
			case '@':  //set
				while (len_old >= 0 && len_old < my_tcp_cache.len)
				{
					for (short i = 0; i < MAX_NAME; i++)
					{
						if (0 <= str1_find_str2_1(my_tcp_cache.data, len_old, str1_find_char_1(my_tcp_cache.data, len_old, my_tcp_cache.len, ':'), str_data_names[i]))
						{
							int value = str1_find_char_1(my_tcp_cache.data, len_old, my_tcp_cache.len, ':'); //获取'：:'相对于 my_tcp_cache.data 的位置
							if (value < 0)
							{
								Serial.printf("get ':' error\n");
								break;
							}
							value = str_to_u16(my_tcp_cache.data + value, my_tcp_cache.len - value); //将赋值的分隔符之后，my_tcp_cache.len之前范围内第一个数据转换成u16
							if (value < 0)
							{
								Serial.printf("get u16 error\n");
								break;
							}
							//这里作为调试用，串口发送很占时间
							Serial.printf("\r\nset id = %d	value= %d ", i, (u16)value);
							set_data_(i, (u16)value);
							time_old_ms = millis();
							break;
						}
					}

					my_tcp_cache.data[len_old] = 0; //清楚标志位的数据
					len_old = str1_find_char_1(my_tcp_cache.data, len_old + 1, my_tcp_cache.len, '@');
					//只识别 @ 类型的数据，get类型的数据一般不会组合发送，舍弃此部分
				}
				//所有的指令已经执行完毕
				brightness_work(); //更新一下光控灯的状态
				//TCP 打包返还自己的状态
				if (back_send_tcp_(client, tcp_send_data, set_databack()) == -1)
				{
					Serial.printf(" 2 error_tcp_sum=%d \r\n", error_tcp_sum++);
					return;
				}
				if (power_save != 0) //实时更新断电记忆的东西
				{
					//file_save_stut();
					Serial.printf(" save_values %d \n", save_values(stut_data_file));
					if (power_save == 1)
					{
						power_save = 0; //如果不清零，则每次更改设置都会被flash记忆，flash擦写能力有限，我调试程序的就不每次擦写了
					}
				}
				break; //跳出 switch
			}
			my_tcp_cache.data[0] = 0;
			my_tcp_cache.len = 0; //因为我没有在其他位置调用数据接收函数，所以我处理完之后全部清除了
		}
		//TCP链接失效
		else if (stat == 0)
		{
			Serial.printf(" 3 error_tcp_sum=%d \r\n", error_tcp_sum++);
			delay(1000);
			return;
		}
	}
}
