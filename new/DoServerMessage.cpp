#include "DoServerMessage.h"
#include "myconstant.h"
#include "myudp.h"
#include "mywarn.h"
#include "mytype.h"
#include "jiantin.h"
#include "savevalues.h"
extern "C"
{
#include "mystr.h"
#include "mytimer.h"
}

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

char tcp_send_data[MAX_TCP_DATA]; // 随用随清，不设置长度数组

int other_timer_loop(struct TcpLinkData *tcp_link_data);

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
		101 发送返回给服务器的tcp数据包失败
*/
int do_tcp_data(struct TcpLinkData *tcp_link_data, struct Tcp_cache *my_tcp_cache,void (*callback)())
{
	const int kERROR_send_tcp = 101;
	const int kERROR_no_error = 0;
	const int kMESSAGE_send_tcp_time_no_flush = 1;
	short stat;
	int tcp_senddata_len = 0;
	unsigned long long tmpuL = 0;
	int tmp_status = 0;
	short len_old = 0;
	long long tmp;
	int tmp1;
	// Serial.printf("回复响应时间：%d \n", micros() - time_old);
	// 如果需要对TCP链接返回的数据进行处理，请在这后面写，-----------------------------------------------------------------------
	// 示例//将TCP返回的数据当作字符串输出
	Serial.print(my_tcp_cache->data);

	switch (*my_tcp_cache->data)
	{
	case 'A': // 新增一个联动监听
		// 尝试使用 '\t' 作为分割符号，一次接收多个监听指令
		tcp_senddata_len = 0;
		tmp1 = str1_find_char_1(my_tcp_cache->data, len_old, my_tcp_cache->len, '\t');
		while (tmp1 > 0)
		{
			my_tcp_cache->data[tmp1] = '\0';
			tmp_status = add_jiantin(my_tcp_cache->data + len_old, tmp1);
			tcp_senddata_len = tcp_senddata_len + sprintf(tcp_send_data + tcp_senddata_len, "L%d", tmp_status);
			len_old = tmp1 + 1;
			tmp1 = str1_find_char_1(my_tcp_cache->data, len_old, my_tcp_cache->len, '\t');
		}
		jiantin_print();
		if (back_send_tcp_(tcp_link_data->client, tcp_send_data, tcp_senddata_len) == -1)
		{
			return kERROR_send_tcp;
		}
		tcp_link_data->send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳
		break;
	case 'C':
		// 重新触发一些监听的事件。好像是把错误状态重置成未触发，清除掉旧的已发送的状态。
		// len_old=0
		tmp1 = 0;
		do
		{
			tmpuL = str_to_u64(my_tcp_cache->data + len_old, my_tcp_cache->len - len_old, &stat);
			len_old = str1_find_char_1(my_tcp_cache->data, len_old + 1, my_tcp_cache->len, 'C');
			if (stat != 1)
			{
				break;
			}
			set_not_warn(tmpuL);
			tmp1 = tmp1 + 1;
		} while (len_old > 0);
		tcp_senddata_len = sprintf(tcp_send_data, "#%d", tmp1);
		if (back_send_tcp_(tcp_link_data->client, tcp_send_data, tcp_senddata_len) == -1)
		{
			return kERROR_send_tcp;
		}
		tcp_link_data->send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳
		break;
	case 'D': // 删除一个联动
		tmpuL = str_to_u64(my_tcp_cache->data, my_tcp_cache->len, &stat);
		if (stat != 1)
		{
			Serial.printf(" D   str_to_u64 error	%d", stat);
			break;
		}
		jiantin_del(tmpuL);
		tcp_senddata_len = sprintf(tcp_send_data, "%s", my_tcp_cache->data);
		if (back_send_tcp_(tcp_link_data->client, tcp_send_data, tcp_senddata_len) == -1)
		{
			return kERROR_send_tcp;
		}
		tcp_link_data->send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳

		break;
	case 'm': // message
	case 'w': // warning
	case 'e': // error
		// 这里是服务器收到udp消息之后的回复
		tmpuL = str_to_u64(my_tcp_cache->data, my_tcp_cache->len, &stat);
		if (stat < 0)
		{
			Serial.printf("  mwe  str_to_u64 error	%d", stat);
			break;
		}
		tcp_senddata_len = warn_ack(tmpuL, (enum UdpMessageClass) * (my_tcp_cache->data + len_old), tcp_send_data); // tmp原来存错误id现在存长度
		if (tcp_senddata_len < 2)																				   // 基础长度两个#号
		{
			// 请求的错误已经消除
			Serial.printf("   loop warn_ack return 0 ");
			break;
		}
		if (back_send_tcp_(tcp_link_data->client, tcp_send_data, tcp_senddata_len) == -1)
		{
			return kERROR_send_tcp;
		}
		tcp_link_data->send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳
		break;
	case 'T':
		// case 't':
		// 这里处理心跳包返回的时间戳，无需返回任何数据
		str_get_time(&Now, my_tcp_cache->data);
		break;
	case '+': // 获取传感器和模式的信息
	case 'G':
	case 'g':
		if (back_send_tcp_(tcp_link_data->client, tcp_send_data, set_databack(COMMAND_FIG, tcp_send_data, MAX_TCP_DATA)) == -1)
		{
			return kERROR_send_tcp;
		}
		tcp_link_data->send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳
		break;										// 跳出 switch
	case 'I':										// 获取一些模式id的详细描述
	case 'i':										// 发送 可选变量描述 MODE_INFO
		if (MODE_INFO == NULL)
		{
			break;
		}
		if (back_send_tcp(tcp_link_data->client, MODE_INFO) == -1)
		{
			return kERROR_send_tcp;
		}
		tcp_link_data->send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳
		break;										// 跳出 switch
	case '@':										// set
		while (len_old >= 0 && len_old < my_tcp_cache->len)
		{
			// 计算查找名字数据分割线的范围，取最小值
			tmp1 = len_old + 50;
			if (my_tcp_cache->len < tmp1)
			{
				tmp1 = my_tcp_cache->len;
			}
			// 查找名字数据之间的分割符号
			int value = str1_find_char_1(my_tcp_cache->data, len_old, tmp1, '['); // 获取 '[' 相对于 my_tcp_cache->data 的位置
			if (value < 0)														 // 限制名字的长度,找不到 '[' 就去找 ':'
			{
				// Serial.printf("get '[' error value= %d %d\n", value, len_old);
				value = str1_find_char_1(my_tcp_cache->data, len_old, tmp1, ':'); // 获取':'相对于 my_tcp_cache->data 的位置
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
				if (1 == str1_eq_str2(my_tcp_cache->data, len_old, value, data_list[i].name))
				{
					value = str1_find_char_1(my_tcp_cache->data, value, my_tcp_cache->len, ':') + 1; // 找到的是 ':' 真正的数据从下一位开始

					if (set_value(data_list + i, my_tcp_cache->data + value, my_tcp_cache->len - value) == 1)
					{
						Serial.printf("set_value ok %d	", i);
					}
					else
					{
						Serial.printf("set_value error %d %s\r\n", i, my_tcp_cache->data + value);
					}
					break;
				}
			}
			my_tcp_cache->data[len_old] = 0; // 清楚标志位的数据
			len_old = str1_find_char_1(my_tcp_cache->data, len_old + 1, my_tcp_cache->len, '@');
			// 只识别 @ 类型的数据，get类型的数据一般不会组合发送，舍弃此部分
		}
		// 所有的指令已经执行完毕
		callback();
		// TCP 打包返还自己的状态
		if (back_send_tcp_(tcp_link_data->client, tcp_send_data, set_databack(COMMAND_FIG, tcp_send_data, MAX_TCP_DATA)) == -1)
		{
			return kERROR_send_tcp;
		}
		tcp_link_data->send_time_old_ms = millis(); // 这里发送了，就没有必要一直发心跳包了，更新一下心跳包的时间戳
		if (power_save != 0)						// 实时更新断电记忆的东西
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
	my_tcp_cache->data[0] = 0;
	my_tcp_cache->len = 0; // 因为我没有在其他位置调用数据接收函数，所以我处理完之后全部清除了
	return kERROR_no_error;
}

int send_hart_back(struct TcpLinkData *tcp_link_data)
{
	return back_send_tcp_(tcp_link_data->client, tcp_send_data, set_databack(HEART_BEAT_FIG, tcp_send_data, MAX_TCP_DATA));
}

/*
一般会阻塞 RUAN_TIMEer_us 等待tcp数据的传输

callback 是在有 “extern struct MyType data_list[MAX_NAME];” 变量被修改的时候会调用

0 服务器没发数据
1 成功处理数据
101 连接断开
*/
int wait_and_do_server_message(struct TcpLinkData *tcp_link_data,void (*callback)())
{
	if (!tcp_link_data->client || !tcp_link_data->client->connected())
	{
		return 101;
	}

	short stat = timeout_back_us(tcp_link_data->client, RUAN_TIMEer_us); // 等待100us tcp是否有数据返回

	if (stat == 1) // 有收到TCP数据
	{
		stat = get_tcp_data(tcp_link_data->client, &my_tcp_cache);
		if (my_tcp_cache.len < 1) // 收到有效字节
		{
			return 0; // 没有收到有效的数据，不用继续往后
		}
		// get_time_old_ms = millis(); // 更新最后一次接收到数据的时间戳
		int error = do_tcp_data(tcp_link_data, &my_tcp_cache,callback);
		if (error != 0)
		{
			Serial.printf("error: file %s,line %d, code %d\r\n", __FILE__, __LINE__, error); // TCP 刚好失效的时候就触发了
			return 101;
		}
	}
	// TCP链接失效
	else if (stat == 0)
	{
		return 101;
	}

	return other_timer_loop(tcp_link_data);
}

int other_timer_loop(struct TcpLinkData *tcp_link_data)
{
	if (millis() - tcp_link_data->ruan_time_old_ms > RUAN_TIMEer_ms)
	{
		if (millis() - tcp_link_data->ruan_time_old_ms > 1000)
		{
			tcp_link_data->ruan_time_old_ms = millis();
			next_sec(&Now);

			// 隔一段时间就发送一次本机数据，心跳包
			if (millis() - tcp_link_data->send_time_old_ms > HEART_BEAT_ms) // 无符号整型的加减运算，就算溢出了，也不影响差的计算，windows gcc 0-0xfffffffe=2;
			{
				if (millis() - tcp_link_data->get_time_old_ms > HEART_BEAT_TIMEOUT_ms)
				{
					// 太久没有收到服务器发来的数据，重新连接服务器
					Serial.printf(" %d not get tcp data ,return\r\n", HEART_BEAT_TIMEOUT_ms);
					return 101;
				}
				Serial.print('#');
				// TCP发送心跳包
				if (send_hart_back(tcp_link_data) == -1)
				{
					return 101;
				}
				// 在这里插入心跳包的返回值检测，超时未回复，就认定链接失效，重新建立tcp的链接
				/*实现方式：添加两个标志？记录发送状态和接收状态，判断发送和接受状态的情况*/
				tcp_link_data->send_time_old_ms = millis();
			}
		}

		// 验证监听数据
		if (jiantin_loop() < 0)
		{
			Serial.printf("error: file %s,line %d, jiantin_loop malloc error\r\n", __FILE__, __LINE__);
		}
		// 发送警告
		warn_send();
		// 调用软定时器
		tcp_link_data->ruan_time_old_ms = millis(); // 更新时间
	}
	return 0;
}

void free_tcp_lick(struct TcpLinkData *tcp_lick_data)
{
	if (tcp_lick_data->client)
	{
		delete tcp_lick_data->client;
		tcp_lick_data->client = nullptr;
	}
}

/*
uid 手机app发过来的用户id，chipID 可以使用ESP.getChipId()读取出来

返回一个结构体，判断结构体的 client 是否为NULL，null则为建立链接失败，等1S在试试，多次失败，重启WiFi
*/
struct TcpLinkData init_server_tcp_link(const char *host, uint16 port, uint64_t uid, uint32_t chipID)
{
	WiFiClient *client;
	client = new WiFiClient();
	struct TcpLinkData tcp_lick_data;
	tcp_lick_data.client = client;

	short stat;
	if (client->connect(MYHOST, TCP_PORT))
	{
		if (uid != 0)
		{
			client->printf("+UID:%llu", uid);
		}

		String str1 = "class_id=1,chip_id=" + String(chipID) + ",ip=" + WiFi.localIP().toString() + ",mac=" + WiFi.macAddress();
		client->print(str1);
	}
	else
	{
		free_tcp_lick(&tcp_lick_data);
		return tcp_lick_data;
	}
	Serial.print("tcp ok");

	stat = timeout_back_ms(client, 3000);
	if (stat == 1)
	{
		// 这里收到的信息可能是服务器返回的第一条信息
		Serial.println(my_tcp_cache.data + get_tcp_data(client, &my_tcp_cache));
		// tmp1 临时储存一下+EID的开始位置
		int tmp1 = str1_find_str2_(my_tcp_cache.data, my_tcp_cache.len, "+EID");
		if (tmp1 >= 0)
		{
			EID = str_to_u64(my_tcp_cache.data + tmp1, my_tcp_cache.len, &stat);
			if (stat != 1)
			{
				// 值转换出错，溢出或未找到有效值//这种可能是服务器数据错误，休眠一会儿等服务器修复
				Serial.printf("error: file %s,line %d, get eid error stat %d\r\n", __FILE__, __LINE__, stat);
				// ESP.deepSleep(20000000, WAKE_RFCAL);
				free_tcp_lick(&tcp_lick_data);
				return tcp_lick_data;
			}

			// 从 my_tcp_cache.data 剩下的数据里提取出时间，校准单片机的时间
			tmp1 = str1_find_char_(my_tcp_cache.data, my_tcp_cache.len, 'T');
			// Serial.println(my_tcp_cache.data + tmp1);
			str_get_time(&Now, my_tcp_cache.data + tmp1);

			char str_EID[22] = {0};
			sprintf(str_EID, "%llu", EID); // 垃圾string(),居然不能装换long long类型的数据，还要我自己动手
			UDP_head_data = "+EID=" + String(str_EID) + ",chip_id=" + String(chipID);
			Serial.print(UDP_head_data);
		}
		my_tcp_cache.len = 0;
	}
	else //  if (stat == 0)
	{
		// 这种可能是服务器数据错误，休眠一会儿等服务器修复
		Serial.printf("error: file %s,line %d, tcp read data timeout\r\n", __FILE__, __LINE__);
		// Serial.print("\r\nservier error,deepSleep\r\n");
		// ESP.deepSleep(20000000, WAKE_RFCAL);
		free_tcp_lick(&tcp_lick_data);
		return tcp_lick_data;
		// return;
	}

	// tcp_lick_data.client = client;
	tcp_lick_data.get_time_old_ms = millis();
	tcp_lick_data.send_time_old_ms = millis();
	tcp_lick_data.last_send_jiantin_ms = millis();
	return tcp_lick_data;
}