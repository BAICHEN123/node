#include "usermain.h"
extern "C"
{
const char *str_data_names[MAX_NAME] = {
	"红外人体检测",
	"烟雾逻辑",
	"烟雾模拟",
	"@开关1[0-1]",
	"@开关2[0-1]",
	"@断电记忆[0-2]"
};
const char *MODE_INFO = "@断电记忆[0-2]:关闭，仅本次，所有";
	
uint8_t power_save = 0; //断电记忆

//两个开关，当他为2时，是自动模式，其他时候读取12 和14号脚的电平
uint8_t LED1 = 0;
uint8_t LED2 = 0;


//下面定义几个引脚的功能
const uint8_t jd1 = 14;			 //1号继电器
const uint8_t jd2 = 12;			 //2号继电器
const uint8_t yan_wu = 13;		 //烟雾逻辑输入
const uint8_t hongwai_renti = 5; //红外人体
const uint8_t anjian1 = 0;		 //按键1输入

void timer1_worker()
{
	clear_wifi_data(wifi_ssid_pw_file); //长按按键1清除wifi账号密码记录
}

void my_init()
{
	pinMode(yan_wu, INPUT);
	pinMode(anjian1, INPUT); //按键1
	pinMode(hongwai_renti, INPUT);
	set_timer1_ms(timer1_worker, TIMER1_timeout_ms); //强制重新初始化定时中断，如果单纯的使用 dht11_get 里的过程初始化，有概率初始化失败

}

void add_values()
{
	add_value(&LED1, sizeof(LED1));
	add_value(&LED2, sizeof(LED2));
}

//每隔 RUAN_TIMEer_ms
void ruan_timer_ms()
{


}

//每隔 RUAN_TIMEer_us
void ruan_timer_us()
{


}

void refresh_work()
{

}

int set_databack(const char fig,char *tcp_send_data)
{
	int i, k, count_char;
	tcp_send_data[0] = fig; //在这里插入开始符号
	tcp_send_data[1] = '#'; //在这里插入开始符号
	count_char = 2;
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
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", digitalRead(hongwai_renti));
			break;
		case 1:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", digitalRead(yan_wu));
			break;
		case 2:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d%%", system_adc_read() * 100 / 1024);
			break;
		case 3:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", LED1);
			break;
		case 4:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", LED2);
			break;
		case 5:
			count_char = count_char + sprintf(tcp_send_data + count_char, "%d", power_save);
			break;
		}
		tcp_send_data[count_char++] = '#'; //在这里插入单个数据结束符
	}
	//Serial.printf("  count_char :%d  ", count_char);
	return count_char;
}


void set_data_(short i, short value)
{
	switch (i)
	{ //在这里修改控件的状态
	case 3:
		if (value > -1 && value < 2)
		{
			LED1 = value;
			digitalWrite(jd1, LED1);
		}
		break;

	case 4:
		if (value > -1 && value < 2)
		{
			LED2 = value;
			digitalWrite(jd2, LED2);
		}
		break;
		
	case 5:
		if (value >= 0 && value <= 2)
		{
			power_save = value;
		}
		break;
	}
}





}

