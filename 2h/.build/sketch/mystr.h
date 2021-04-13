#ifndef _MYSTR_h
#define _MYSTR_h

#include "arduino.h"
/*
字符串转u16
IPD, 10:12345678->10
把传入的第一个自然数转换成u16
*/
u64 str_to_u64(char* str1, unsigned int len);
/*
字符串转long long
IPD, 10:12345678->10
把传入的第一个自然数数转换成long long
str1:字符串指针的开始位置
len:需要查找的数组长度
*/
int str_to_u16(char* str1);
/*
字符数组查找字符串
str1:数据
str_length:数据的长度或需要查找的范围
str2:需要找到的字符串
查找成功：返回str2在str1中的开始位置
查找失败：返回-1

*/
int str1_find_str2_1(char* str1, int start_i, int str_length, char* str2);
/*
//字符串查找字符串
str1:数据
start_i:查找开始位置
str_length:数据的长度或需要查找的范围
str2:需要找到的字符串
str1
|-----str2----------------|
   ^  ^               ^
   |start_i           |str_length
	  |return

*/
int str1_find_str2_(char* str1, int str_length, char* str2);
/*
字符数组查找字符
str1:数据
str_length:数据的长度或需要查找的范围
char1:需要找到的字符
查找成功：char* str1 的角标
查找失败：返回-1

*/
int str1_find_char_(char* str1, int str_length, char char1);
/*
字符数组查找字符
str1:数据
start_x:数据查找的开始位置
str_length:数据的长度或需要查找的范围
char1:需要找到的字符
查找成功：char* str1 的角标
查找失败：返回-1
*/
int str1_find_char_1(char* str1, int start_x, int str_length, char char1);
#endif

