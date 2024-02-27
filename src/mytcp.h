#ifndef _MYTCP_H
#define _MYTCP_H

#include <WiFiClient.h>
#include "myconstant.h"
extern "C"
{
    struct Tcp_cache
    {
        char data[MAX_TCP_DATA];
        int len;
        const int LEN_MAX = MAX_TCP_DATA;
    };
    extern struct Tcp_cache my_tcp_cache; // TCP缓存数组
    short timeout_back_ms(WiFiClient *client, unsigned short timeout_ms_max);
    short timeout_back_us(WiFiClient *client, unsigned short timeout_us_max);
    short get_tcp_data(WiFiClient *client, struct Tcp_cache *tcp_data);
    short back_send_tcp_of_type(WiFiClient *client, unsigned char type, const char *tcp_send_data, int len);
    // short back_send_tcp_(WiFiClient *client, char *tcp_send_data, int len);
    // short back_send_tcp(WiFiClient *client, const char *str1);
    // u32 save_tcp_data_in_file(WiFiClient *client, const char *file_name, u32 file_len);
}
#endif