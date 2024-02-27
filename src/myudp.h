#ifndef _MYUDP_H
#define _MYUDP_H
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include "myconstant.h"
extern "C"
{
    short UDP_Send(const char *UDP_IP, uint16_t UDP_port, String udp_send_data);
}

#endif