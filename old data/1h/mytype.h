#ifndef __MYTYPE_H
#define __MYTYPE_H
#include "arduino.h"
#include "jiantin.h"
extern "C"
{
#ifndef NULL
#define NULL (void *)0
#endif
#include "mystr.h"
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
		//TYPE_STR_N //char[]	使用 strcpy 进行复制，到'\0'或者长度限制 MyType.byte_len 结束

	};
	struct MyType
	{
		/* data */
		const char *name; //字节长度不要超过50，不然服务器拒收
		const char *unit;
		const enum type_id ID;
		unsigned int byte_len; //这个长度是指 data 被分配的字节长度
		void *data;
		void *min; //数据的最小值
		void *max; //数据的最大值
	};
	int mystrcpy(char *str1, char *str2, int len1, int len2);
	//int get_data_str(struct MyType *mytype, char *data, int len_data);
	//int get_data_str(void *mytype_data, const enum type_id ID, char *data, int len_data);
	int get_data_str(void *mytype_data,int len,const enum type_id ID, char *data, int len_data);
	int get_data_unit_str(struct MyType *mytype, char *data, int len_data);
	int get_name_str(struct MyType *mytype, char *data, int len_data);
	int set_value(struct MyType *mytype, char *data, int len_data);

	/*传入字符串和数据类型，将字符串内装换成对应的数据类型
	**************************************************注意：这个函数会申请内存，不用的时候记得释放**********************
	失败：return NULL
	成功：return 申请到的储存转换结果的地址
	malloc_len 请传入变量的地址，将会返回malloc申请到的长度

	*/
	void *get_value(char *data,int len_data, const enum type_id ID, unsigned char *malloc_len);

	/*将数据拿来做比较
	1 符合 data1  fuhao  data2
	0 不符合，或者有错误
	*/
	int is_true(const enum type_id ID, void *data1, char fuhao, void *data2);
}
#endif