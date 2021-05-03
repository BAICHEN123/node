#ifndef _SAVEVALUES_H
#define _SAVEVALUES_H

#include <Arduino.h>
#include <LittleFS.h>
extern "C"
{
#define list_values_len_max 10 //需要存储多少个变量的值

    int get_list_values_len();
    /*如果要保存一个数组，len请输入数组的字节长度*/
    int add_value(void *data, int len);
    /*将数据保存到文件*/
    int save_values(const char *file_name);
    /*从文件中读出数据*/
    int read_values(const char *file_name);
    

}
#endif