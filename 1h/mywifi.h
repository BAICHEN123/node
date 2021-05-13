
#ifndef _MYWIFI_h
#define _MYWIFI_h

#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
extern "C"
{
#include "myconstant.h"
#include "mystr.h"
#ifndef SERVER_CLIENT_PROT
//做服务器时监听的端口
#define SERVER_CLIENT_PROT 9997
#endif

//wifi 的名称密码长度限制
#ifndef WIFI_ssid_len
#define WIFI_ssid_len 32
#endif

#ifndef WIFI_password_len
#define WIFI_password_len 32
#endif
    static const char *WIFI_SERVER_NAME = "HCC_APP";
    static const char *WIFI_SERVER_PASSWORD = "12345678";

    void set_anjian1(const uint8_t pin);
    short file_read_wifidata(char *WIFI_ssid, char *WIFI_password, const char *wifi_ssid_pw_file);
    short file_save_wifidata(char *WIFI_ssid, char *WIFI_password, const char *wifi_ssid_pw_file);
    short file_delete(const char *file_name);
    short get_wifi(char *WIFI_ssid, char *WIFI_password, const char *wifi_ssid_pw_file);
    short tcp_server_get_wifi_data(char *WIFI_ssid, char *WIFI_password, unsigned long &UID, uint32_t CHIP_ID, const char *wifi_ssid_pw_file);
    void clear_wifi_data(const char *file_name);
}
#endif
