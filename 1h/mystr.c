#include "mystr.h"


/*
字符串转u16
IPD, 10:12345678->10
把传入的第一个自然数转换成u16
*/
int str_to_u16(char* str1)
{
	int i;
	while (*(str1) < '0' || *(str1) > '9')
	{
		str1++;
	}
	i = (*str1) & 15;//&207（1100 1111）  char 转 int
	if (*(str1 - 1) == '-')
	{
		i = -1 * i;
	}
	str1++;
	while ((*str1 > 47) && (*str1 < 58))
	{
		i = i * 10 + (*str1 & 15);
		str1++;
	}
	return i;
}
/*
字符串转long long
IPD, 10:12345678->10
把传入的第一个自然数数转换成long long
str1:字符串指针的开始位置
len:需要查找的数组长度
*/
u64 str_to_u64(char* str1,unsigned int len)
{
	long long i;
	u64 old_q = str1;
	while (*(str1) < '0' || *(str1) > '9')
	{
		str1++;
		if (str1 - old_q > len)
		{
			break;
		}
	}
	i = (*str1) & 15;//&207（1100 1111）  char 转 int
	if (*(str1 - 1) == '-')
	{
		i = -1 * i;
	}
	str1++;
	while ((*str1 > 47) && (*str1 < 58))
	{
		i = i * 10 + (*str1 & 15);
		str1++;
	}
	return i;
}

/*
字符数组查找字符串
str1:数据
str_length:数据的长度或需要查找的范围
str2:需要找到的字符串
查找成功：返回str2在str1中的开始位置
查找失败：返回-1

*/
int str1_find_str2_(char* str1, int str_length, char* str2)
{
	int str1_i = 0;
	int str2_i = 0;
	while (str1_i <= str_length)//str1没有结束
	{
		while ((*(str2 + str2_i) != '\0') && ((str1_i + str2_i) <= str_length) && (*(str1 + str1_i + str2_i) == *(str2 + str2_i)))//str2没有结束 && 未溢出 && 有相同的字符
		{
			str2_i = str2_i + 1;//str2_i+1开始比对下一个字符
		}
		if (*(str2 + str2_i) == '\0')//str2比对结束,全部相同
		{
			return str1_i;//返回字符串相同的str1开始位置
		}
		else
		{
			str1_i++;//比对失败，从下一个字节开始重新比对
			str2_i = 0;
		}
	}
	return -1;//比对失败
}

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
int str1_find_str2_1(char* str1, int start_i, int str_length, char* str2)
{

	int str1_i = start_i;
	int str2_i = 0;
	while (str1_i < str_length)//str1没有结束
	{
		while ((*(str2 + str2_i) != '\0') && ((str1_i + str2_i) < str_length) && (*(str1 + str1_i + str2_i) == *(str2 + str2_i)))//str2没有结束 && 未溢出 && 有相同的字符
		{
			str2_i = str2_i + 1;//str2_i+1开始比对下一个字符
		}
		if (*(str2 + str2_i) == '\0')//str2比对结束,全部相同
		{
			return str1_i;//返回字符串相同的str1开始位置
		}
		else
		{
			str1_i++;//比对失败，从下一个字节开始重新比对
			str2_i = 0;
		}
	}
	return -1;//比对失败
}

/*
字符数组查找字符
str1:数据
str_length:数据的长度或需要查找的范围
char1:需要找到的字符
查找成功：char* str1 的角标
查找失败：返回-1

*/
int str1_find_char_(char* str1, int str_length, char char1)
{
	int str1_i = 0;
	while (str1_i <= str_length)//str1没有结束
	{
		if (str1[str1_i] == char1)
		{
			return str1_i;
		}
		str1_i++;//比对失败，从下一个字节开始重新比对
	}
	return -1;//比对失败
}

/*
字符数组查找字符
str1:数据
start_x:数据查找的开始位置
str_length:数据的长度或需要查找的范围
char1:需要找到的字符
查找成功：char* str1 的角标
查找失败：返回-1
*/
int str1_find_char_1(char* str1, int start_x,int str_length, char char1)
{
	while (start_x <= str_length)//str1没有结束
	{
		if (str1[start_x] == char1)
		{
			return start_x;
		}
		start_x++;//比对失败，从下一个字节开始重新比对
	}
	return -1;//比对失败
}