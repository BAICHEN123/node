#ifndef __JIANTIN_H
#define __JIANTIN_H
#include "Arduino.h"
#include "mywarn.h"
#include "mytype.h"
#include "myconstant.h"

/*
处理流程

1、连接上服务器之后，由服务器发来监听数据，分段后使用 add_jiantin 添加到监听队列中去
2、在主循环中每隔一定时间调用这里的一个函数
	该函数需要完成：
					1、监听的判定
					2、打包一个 'm' 类型的警告，将该监听条目的bid发送给服务器（务必在字符串的结尾加'\0'）
					3、回收警告申请的内存，警告和警告中字符串所占的内存都要回收

*/
extern "C"
{
	struct JianTin
	{
		//注意，声明变量的顺序，会改变此类型的长度，他要四字节对齐
		unsigned long long sql_id; //储存此条目在数据库里面的id
		int name_id;			   //储存需要监听的条目在 data_list 的索引号
		char *name;				   //储存名字
		void *data;				   //储存比较用的数据
		struct Udpwarn *warn;
		unsigned char name_len;
		unsigned char data_len;
		char fuhao; //储存此条目比较时用到的符号
	};

//数值的字节长度	%30s
#define CANZHI_MAX_LEN 30
//名字的字节长度	%50s
#define NAME_MAX_LEN 50
//warn 内存的暂存时间 单位 ms
#define WARN_DELETE_OUTTIME_MS 30*1000

#ifndef JianTin_MAX_LEN
#define JianTin_MAX_LEN 20
#endif

	//从字符串内读取数据，添加一个监听事件
	int add_jiantin(char *tcp_data, int data_len);
	int jiantin_loop();
	void jiantin_print();
	/*删除一个监听的要求，释放内存，移动位置，
	释放 warn 指向的内存，释放 warn 的内存，释放只想 warn 的内存，释放 JianTin 内存
	*/
	void jiantin_del(unsigned long long sql_id);

	/*
	为了将一个已经触发并收到回应的监听重新触发，将指定 fid 对应的监听，设置为未触发
	*/
	void set_not_warn(unsigned long long sql_id);
}
#endif