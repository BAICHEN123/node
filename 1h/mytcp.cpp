#include "mytcp.h"

extern "C"
{

	/*
TCP阻塞，等待 timeout_ms_max ms
看看有没有TCP包返回回来，不会自动断开链接
返回值
0，TCP失效了
1，等到了TCP包
2，没有等到，超时
*/
	short timeout_back_ms(WiFiClient client, unsigned short timeout_ms_max)
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
	short get_tcp_data(WiFiClient client, struct Tcp_cache *tcp_data)
	{
		short start_id = tcp_data->len;
		while (client.available() && tcp_data->len < MAX_TCP_DATA)
		{
			// if (tcp_data->len >= MAX_TCP_DATA)
			// {
			// 	return -1;
			// }
			tcp_data->data[tcp_data->len++] = static_cast<char>(client.read());
		}
		tcp_data->data[tcp_data->len] = '\0';
		return start_id;
	}

	/*将打包好的数据，用TCP发送出去*/
	short back_send_tcp_(WiFiClient client, char *tcp_send_data, int len)
	{
		if (client.connected()) //函数第一次执行loop循环的时候这里可能会出错，因为 client 第一次赋值为局部变量，在setup 中修改他的初始化就可以了
		{
			//在这里合成需要发送出去的传感器数据？
			client.write(tcp_send_data, len);
			client.flush(3000);//限制等待时间
			return 1;
		}
		else
		{
			//结束此次 loop ，到开始位置，重新连接TCP
			client.stop();
			return -1;
		}
	}

	/*将打包好的数据，用TCP发送出去*/
	short back_send_tcp(WiFiClient client, const char *str1)
	{
		if (client.connected()) //函数第一次执行loop循环的时候这里可能会出错，因为 client 第一次赋值为局部变量，在setup 中修改他的初始化就可以了
		{
			//在这里合成需要发送出去的传感器数据？
			client.write(str1);
			client.flush();
			return 1;
		}
		else
		{
			//结束此次 loop ，到开始位置，重新连接TCP
			client.stop();
			return -1;
		}
	}
}