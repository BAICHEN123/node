#ifndef _MYWARN_H
#define _MYWARN_H
//extern
/*文件作用
使用一个数组来记录所有的报错信息，或者报错信息的格式化字符串

报错信息分类为  udp 分包
	格式要求	"^\+EID=(\d+),chip_id=(\d+)([mwe])(\d+),(.+)$"
	区别不同内容的udp包
	消息，警告，错误		mesg，warn，erro		m,w,e//只是用第一个字节来表示包的性质
	m:设备之间联动指令
	w:有传感器数据超出范围，给予设备绑定了的用户邮件警告
	e:设备异常，告知用户的同时向服务器留下日志
	
	数组要求
	需要包含一个字符串，一个id，id可以用数组的id作为报错id，然后使用一个字节来记录分包类型
方案：
	当在定时中断收到异常之后
	1、在原地调用 set_warn 并填入对用的id
	2、软中断 ms级别的去检查错误，然后逐个发送
		关于发送：
			1、发送的时候根据id找到 udpwarn 中的位置，并修改 struct Udpwarn 类型的属性
			2、tcp 处理好  struct Udpwarn  其他属性
	3、清除 warn_id 标记
	4、


*/
#include <Arduino.h>
#include "myconstant.h"
#include "myudp.h"
extern "C"
{
enum UdpMessageClass
{
	MESSAGE='m',WARN='w',ERROR='e'
};
enum WarnType
{
	NOT_WARN=0,//未触发
	IS_WARN,//刚刚触发
	WAIT_SEND,//等待发送
	WAIT_ACK,//等待响应
	ACK,//已响应
	TIMEOUT//超时未响应
};
struct Udpwarn
{
	enum UdpMessageClass cmsg;//记录此条内容的警告级别
	enum WarnType status;//记录和服务器的交互状态
	unsigned long time;//记录时间
	unsigned int id;//记录报错的 id 号
	const char* str_waring;//要告知用户的话
};


#define WARN_LEN 3
#define UDP_TIME_OUT_MS 3000 
//extern int len_warn_id=0;

//int warn_exist(struct Udpwarn *warn);
int set_warn(struct Udpwarn *warn);
void warn_send();
void warn_ack(unsigned int id);



}
#endif

