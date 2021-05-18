#include "mystr.h"

//风险1，如果没有查找到 需要的内容 或者 '\0' 会一直沿着地址查找，调用的时候注意一下，尽量不要使用没有止境的查找
//风险2，如果字符串内有一长段的数字，那么会导致溢出，目前仅有 str_to_u64 对溢出有判断

/*
字符串转u16
IPD, 10:12345678->10
把传入的第一个自然数转换成u16
传出正数则装换成功
创出负数则转换失败，传回的复数是最后转换的有效位
*/
int str_to_u16(const char *str1, unsigned int len)
{
    int i;
    char *q_str1 = (char *)str1;
    while (*(q_str1) < '0' || *(q_str1) > '9')
    {
        q_str1++;
        if (q_str1 - str1 > len)
        {
            return -1;
        }
    }
    i = (*q_str1) & 15; //&207（1100 1111）  char 转 int
    q_str1++;
    while ((*q_str1 > 47) && (*q_str1 < 58) && (q_str1 - str1 < len))
    {
        /* 
        65535
        6552 后面可以跟 0-9
        6553 后面只能跟 0-5
        再大就会溢出
        */
        if (i < 6553)
        {
            i = i * 10 + (*q_str1 & 15);
            q_str1++;
        }
        else if (i <= 6553 && *q_str1 <= '5')
        {

            i = i * 10 + (*q_str1 & 15);
            q_str1++;
        }
        else
        {
            return -1 * i;
        }
    }
    return i;
}
/*
字符串转unsigned long long
IPD, 10:12345678->10
把传入的第一个自然数数转换成unsigned long long
str1:字符串指针的开始位置
len:需要查找的数组长度

status:	-1	在长度范围内没有找到任何一个数字
			return:	0
		-2	转换过程溢出
			return:	溢出前最后的有效值
		//0	极限反杀	//调试的时候用过，这个留念用的
		1	成功转换
			return:	有效值
*/
u64 str_to_u64(const char *str1, unsigned int len, short *status)
{
    u64 i;
    char *old_q = (char *)str1;
    while (*(old_q) < '0' || *(old_q) > '9') //从 '0' 开始转换数字没有意义  &&  //从 1开始转换，不然后面统计位数会出错
    {
        old_q++;
        if (old_q - str1 > len)
        {
            *status = -1;
            return 0;
        }
    }
    i = (*old_q) & 15; //&207（1100 1111）  char 转 int
    old_q++;
    while ((*old_q > 47) && (*old_q < 58) && (old_q - str1 < len))
    {
        /* 
        1844674407370955160 后面可以跟 0-9
        1844674407370955161 后面只能跟 0-5
        再大就会溢出
        */
        if (i < 1844674407370955161llu)
        {
            i = i * 10 + (*old_q & 15);
            old_q++;
        }
        else if (i <= 1844674407370955161llu && *old_q <= '5')//18446744073709551615
        {
            
            i = i * 10 + (*old_q & 15);
            old_q++;
        }
        else
        {
            *status = -2;
            return i;
        }
    }
    *status = 1;
    return i;
}

/*
字符串转long long
IPD, 10:12345678->10
把传入的第一个自然数数转换成long long
str1:字符串指针的开始位置
len:需要查找的数组长度
*/
// long long str_to_64(char* str1,unsigned int len,short *status)
// {
// 	long long i;
// 	u64 old_q = str1;
// 	while (*(str1) < '0' || *(str1) > '9')
// 	{
// 		str1++;
// 		if (str1 - old_q > len)
// 		{
// 			*status=-1;
// 			return 0;
// 			//break;
// 		}
// 	}
// 	i = (*str1) & 15;//&207（1100 1111）  char 转 int
// 	if (*(str1 - 1) == '-')
// 	{
// 		i = -1 * i;
// 	}
// 	str1++;
// 	while ((*str1 > 47) && (*str1 < 58))
// 	{
// 		i = i * 10 + (*str1 & 15);
// 		str1++;
// 	}
// 	return i;
// }


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
int str1_find_str2_1(char* str1, int start_i, int str_length, const char* str2)
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