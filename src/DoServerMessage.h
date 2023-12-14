#ifndef __DoServerMessage_H
#define __DoServerMessage_H

#include "mytcp.h"

/*
或许我可以搞个结构体，储存一个tcp链接所有相关的数据，就不用担心传递变量很麻烦了
*/

extern "C"
{

	struct TcpLinkData
	{
		WiFiClient *client;
		unsigned long get_time_old_ms;		//= millis();
		unsigned long send_time_old_ms;		// = millis();
		unsigned long last_send_jiantin_ms; // = millis();
		unsigned long time_flush_1s;		//
		unsigned long ruan_time_old_ms;		// 每隔 RUAN_TIMEer_ms 刷新一次监听的数据用的，储存时间戳
	};
	struct TcpLinkData init_server_tcp_link(const char *host, uint16 port, uint64_t uid, uint32_t chipID);
	int wait_and_do_server_message(struct TcpLinkData *tcp_link_data, void (*callback)(), void (*set_value_of)(int index_of_data_list));
	void free_tcp_lick(struct TcpLinkData *tcp_lick_data);
}

#endif