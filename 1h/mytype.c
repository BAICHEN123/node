#include "mytype.h"

//如果那天发现条目少了一般的字符，可能是这个函数的问题
int mystrcpy(char *str1, char *str2, int len1, int len2)
{
	int len = 0;
	while (len < len1 && len < len2 && str2[len] != '\0')
	{
		str1[len] = str2[len];
		len = len + 1;
	}
	return len;
}

/*将 struct MyType mytype 内的 mytype.data 转换成char[]
填充到 char[] 里去
返回填充进去的长度
请自行确保 data 剩余空间至少30字节，因为我关于数字转换的地方都没有确认
只对字符串的复制进行了确认
*/
int get_data_str(struct MyType *mytype, char *data, int len_data)
{
	switch (mytype->ID)
	{
	case TYPE_CHAR:
		return sprintf(data, "%c", *(char *)(mytype->data));
	case TYPE_INT8:
		return sprintf(data, "%d", *(char *)(mytype->data));
	case TYPE_u8:
		return sprintf(data, "%u", *(unsigned char *)(mytype->data));
	case TYPE_SHORT:
		return sprintf(data, "%d", *(short *)(mytype->data));
	case TYPE_USHORT:
		return sprintf(data, "%u", *(unsigned short *)(mytype->data));
	case TYPE_INT:
		return sprintf(data, "%d", *(int *)(mytype->data));
	case TYPE_UINT:
		return sprintf(data, "%u", *(unsigned int *)(mytype->data));
	case TYPE_INT64:
		return sprintf(data, "%lld", *(long long *)(mytype->data));
	case TYPE_U64:
		return sprintf(data, "%llu", *(unsigned long long *)(mytype->data));
	case TYPE_FLOAT:
		return sprintf(data, "%.2f", *(float *)(mytype->data));
	case TYPE_DOUBLE:
		return sprintf(data, "%.2f", *(double *)(mytype->data));
	//case TYPE_CHAR_N:
	//	return sprintf(data, "%s", (char *)(mytype->data));
	case TYPE_STR_N:
		return mystrcpy(data, (char *)(mytype->data), len_data, mytype->byte_len);
	default:
		return -1;
	}
}

int get_data_unit_str(struct MyType *mytype, char *data, int len_data)
{
	int len = 0;
	len = get_data_str(mytype, data, len_data);
	len = len + sprintf(data + len, "%s", mytype->unit);
	return len;
}
