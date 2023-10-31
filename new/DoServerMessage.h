#ifndef __DoServerMessage_H
#define __DoServerMessage_H

#include "mytcp.h"

/*
或许我可以搞个结构体，储存一个tcp链接所有相关的数据，就不用担心传递变量很麻烦了
*/


extern "C"
{
struct TcpLinkData;
// int set_databack(const char fig, char *tcp_send_data, int max_len);
int do_tcp_data(struct Tcp_cache my_tcp_cache,unsigned long *send_time_old_ms,WiFiClient *client);
int send_hart_back(WiFiClient *client);
int wait_and_do_server_message(WiFiClient *client,unsigned long *send_time_old_ms);
}

#endif