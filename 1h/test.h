#ifndef _TEST_h
#define _TEST_h
//#include <Arduino.h>

#include <LittleFS.h>
extern "C"
{
#define WIFI_ssid_len 32
#define WIFI_password_len 32
char WIFI_ssid[WIFI_ssid_len] = { '\0' };
char WIFI_password[WIFI_password_len] = { '\0' };

void get_files_name();
}
#endif

