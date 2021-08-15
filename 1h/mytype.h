#ifndef __MYTYPE_H
#define __MYTYPE_H
#include "arduino.h"
#ifndef NULL
#define NULL (void*)0
#endif

// #ifndef __null
// #define __null (void*)0
// #endif
enum type_id
{
	TYPE_CHAR = 1, //8  输出一个字符
	TYPE_INT8,	   //8	输出7bit对应的数据，1bit的符号位
	TYPE_u8,	   //u8
	TYPE_SHORT,	   //16
	TYPE_USHORT,   //u16
	TYPE_INT,	   //32
	TYPE_UINT,	   //u32
	TYPE_INT64,	   //64
	TYPE_U64,	   //u64
	TYPE_FLOAT,	   //float
	TYPE_DOUBLE,   //double
	//TYPE_CHAR_N,	//char[]	使用逐字节复制的方式，'\0'也会复制，复制好多个，长度由 MyType.byte_len 决定。但是我服务器不打算接收这类数据，所以放弃
	TYPE_STR_N //char[]	使用 strcpy 进行复制，到'\0'或者长度限制 MyType.byte_len 结束

};
struct MyType
{
	/* data */
	const char *name;//字节长度不要超过50，不然服务器拒收
	const char *unit;
	const enum type_id ID;
	unsigned int byte_len; //这个长度是指 data 被分配的字节长度
	void *data;
	void *min;//数据的最小值
	void *max;//数据的最大值
};
int mystrcpy(char *str1, char *str2, int len1, int len2);
int get_data_str(struct MyType *mytype, char *data, int len_data);
int get_data_unit_str(struct MyType *mytype, char *data, int len_data);
int get_name_str(struct MyType *mytype, char *data, int len_data);
#endif