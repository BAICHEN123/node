#ifndef __JIANTIN_H
#define __JIANTIN_H
//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
#include "Arduino.h"
#include "mytype.h"
#include "usermain.h"
extern "C"
{
	struct JianTin
	{
		//注意，声明变量的顺序，会改变此类型的长度，他要四字节对齐
		unsigned long long sql_id; //储存此条目在数据库里面的id
		int name_id;			   //储存需要监听的条目在 data_list 的索引号
		char *name;				   //储存名字
		void *data;				   //储存比较用的数据
		unsigned char name_len;
		unsigned char data_len;
		char fuhao; //储存此条目比较时用到的符号
	};
//%30s
#define CANZHI_MAX_LEN 30
//%50s
#define NAME_MAX_LEN 50

	//从字符串内读取数据，添加一个监听事件
	int add_jiantin(char *tcp_data, int data_len);

	void jiantin_print();
}
#endif