#ifndef _MYTCP_H
#define _MYTCP_H

#include <WiFiClient.h>

extern "C"
{
    #include "myconstant.h"
    struct Tcp_cache
    {
        char data[MAX_TCP_DATA];
        int len;
        const int LEN_MAX=MAX_TCP_DATA;
    };
    short timeout_back_ms(WiFiClient client, unsigned short timeout_ms_max);
    short timeout_back_us(WiFiClient client, unsigned short timeout_us_max);
    short get_tcp_data(WiFiClient client,struct Tcp_cache *tcp_data);
}

#endif