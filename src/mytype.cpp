#include "mytype.h"
// 如果那天发现条目少了一般的字符，可能是这个函数的问题
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
len_data 只有在数据类型是字符串类型的时候才会限制复制字符串的长度
*/
int get_data_str(void *mytype_data, int len, const enum type_id ID, char *data, int len_data)
{
	switch (ID)
	{
	case TYPE_CHAR:
		return sprintf(data, "%c", *(char *)(mytype_data));
	case TYPE_INT8:
		return sprintf(data, "%d", *(char *)(mytype_data));
	case TYPE_u8:
		return sprintf(data, "%u", *(unsigned char *)(mytype_data));
	case TYPE_SHORT:
		return sprintf(data, "%d", *(short *)(mytype_data));
	case TYPE_USHORT:
		return sprintf(data, "%u", *(unsigned short *)(mytype_data));
	case TYPE_INT:
		return sprintf(data, "%d", *(int *)(mytype_data));
	case TYPE_UINT:
		return sprintf(data, "%u", *(unsigned int *)(mytype_data));
	case TYPE_INT64:
		return sprintf(data, "%lld", *(long long *)(mytype_data));
	case TYPE_U64:
		return sprintf(data, "%llu", *(unsigned long long *)(mytype_data));
	case TYPE_FLOAT:
		return sprintf(data, "%.2f", *(float *)(mytype_data));
	case TYPE_DOUBLE:
		return sprintf(data, "%.2lf", *(double *)(mytype_data));
		// case TYPE_CHAR_N:
		//	return sprintf(data, "%s", (char *)(mytype_data));
		//  case TYPE_STR_N:
		//  	return mystrcpy(data, (char *)(mytype_data), len_data, len);
	}
	return -1;
}

/*将 struct MyType mytype 内的 mytype.name 转换成char[]
填充到 char[] 里去
返回填充进去的长度
请自行确保 data 剩余空间至少50字节，因为我关于数字转换的地方都没有确认
*/
int get_name_str(struct MyType *mytype, char *data, int len_data)
{
	int len = 0;
	// if (mytype->name[0] == '@')
	if (mytype->min != NULL && mytype->max != NULL)
	{
		switch (mytype->ID)
		{
		case TYPE_CHAR:
			return sprintf(data, "%s[%c-%c]", mytype->name, *(char *)(mytype->min), *(char *)(mytype->max));
		case TYPE_INT8:
			return sprintf(data, "%s[%d-%d]", mytype->name, *(char *)(mytype->min), *(char *)(mytype->max));
		case TYPE_u8:
			return sprintf(data, "%s[%u-%u]", mytype->name, *(unsigned char *)(mytype->min), *(unsigned char *)(mytype->max));
		case TYPE_SHORT:
			return sprintf(data, "%s[%d-%d]", mytype->name, *(short *)(mytype->min), *(short *)(mytype->max));
		case TYPE_USHORT:
			return sprintf(data, "%s[%u-%u]", mytype->name, *(unsigned short *)(mytype->min), *(unsigned short *)(mytype->max));
		case TYPE_INT:
			return sprintf(data, "%s[%d-%d]", mytype->name, *(int *)(mytype->min), *(int *)(mytype->max));
		case TYPE_UINT:
			return sprintf(data, "%s[%u-%u]", mytype->name, *(unsigned int *)(mytype->min), *(unsigned int *)(mytype->max));
		case TYPE_INT64:
			return sprintf(data, "%s[%lld-%lld]", mytype->name, *(long long *)(mytype->min), *(long long *)(mytype->max));
		case TYPE_U64:
			return sprintf(data, "%s[%llu-%llu]", mytype->name, *(unsigned long long *)(mytype->min), *(unsigned long long *)(mytype->max));
		case TYPE_FLOAT:
			return sprintf(data, "%s[%.2f-%.2f]", mytype->name, *(float *)(mytype->min), *(float *)(mytype->max));
		case TYPE_DOUBLE:
			return sprintf(data, "%s[%.2lf-%.2lf]", mytype->name, *(double *)(mytype->min), *(double *)(mytype->max));
		// case TYPE_CHAR_N:
		//	return sprintf(data, "%s", (char *)(mytype->data));
		// case TYPE_STR_N:
		//	return mystrcpy(data, (char *)(mytype->data), len_data, mytype->byte_len);
		default:
			return -1;
		}
	}
	else
	{
		return sprintf(data, "%s", mytype->name);
	}
}

int get_data_unit_str(struct MyType *mytype, char *data, int len_data)
{
	int len = 0;
	len = get_data_str(mytype->data, mytype->byte_len, mytype->ID, data, len_data);
	if (mytype->unit != NULL)
	{
		len = len + sprintf(data + len, "(%s)", mytype->unit);
	}
	return len;
}

/*
传入的data必须是字符串的开始位置，可以直接data[0]访问
*/

int set_value(struct MyType *mytype, char *data, int len_data)
{

	short status = 0;
	long long value1;
	unsigned long long value2;
	double value3;
	switch (mytype->ID)
	{
	case TYPE_CHAR:
		if (*(char *)(mytype->min) <= data[0] && data[0] <= *(char *)(mytype->max))
		{
			*(char *)(mytype->data) = data[0];
			return 1;
		}
		return 0;
	case TYPE_INT8:
		value1 = str_to_64(data, len_data, &status);
		if (status != 1)
		{
			return 0;
		}
		if (*(char *)(mytype->min) <= value1 && value1 <= *(char *)(mytype->max))
		{
			*(char *)(mytype->data) = (char)value1;
			return 1;
		}
		return 0;
	case TYPE_u8:
		value1 = str_to_64(data, len_data, &status);
		if (status != 1)
		{
			return 0;
		}
		if (*(unsigned char *)(mytype->min) <= value1 && value1 <= *(unsigned char *)(mytype->max))
		{
			*(unsigned char *)(mytype->data) = (unsigned char)value1;
			return 1;
		}
		return 0;
	case TYPE_SHORT:
		value1 = str_to_64(data, len_data, &status);
		if (status != 1)
		{
			return 0;
		}
		if (*(short *)(mytype->min) <= value1 && value1 <= *(short *)(mytype->max))
		{
			*(short *)(mytype->data) = (short)value1;
			return 1;
		}
		return 0;
	case TYPE_USHORT:
		value1 = str_to_u16(data, len_data);
		if (value1 < 0)
		{
			return 0;
		}
		if (*(unsigned short *)(mytype->min) <= value1 && value1 <= *(unsigned short *)(mytype->max))
		{
			*(unsigned short *)(mytype->data) = (unsigned short)value1;
			return 1;
		}
		return 0;
	case TYPE_INT:
		value1 = str_to_64(data, len_data, &status);
		if (status != 1)
		{
			return 0;
		}
		if (*(int *)(mytype->min) <= value1 && value1 <= *(int *)(mytype->max))
		{
			*(int *)(mytype->data) = (int)value1;
			return 1;
		}
		return 0;
	case TYPE_UINT:
		value2 = str_to_u64(data, len_data, &status);
		if (status < 0)
		{
			return 0;
		}
		if (*(unsigned int *)(mytype->min) <= value2 && value2 <= *(unsigned int *)(mytype->max))
		{
			*(unsigned int *)(mytype->data) = (unsigned int)value2;
			return 1;
		}
		return 0;
	case TYPE_INT64:
		value1 = str_to_64(data, len_data, &status);
		if (status != 1)
		{
			return 0;
		}
		if (*(long long *)(mytype->min) <= value1 && value1 <= *(long long *)(mytype->max))
		{
			*(long long *)(mytype->data) = value1;
			return 1;
		}
		return 0;
	case TYPE_U64:
		value2 = str_to_u64(data, len_data, &status);
		if (status < 0)
		{
			return 0;
		}
		if (*(unsigned long long *)(mytype->min) <= value2 && value2 <= *(unsigned long long *)(mytype->max))
		{
			*(unsigned long long *)(mytype->data) = value2;
			return 1;
		}
		return 0;
	case TYPE_FLOAT:
		value3 = 0;
		status = (short)sscanf(data, "%lf", &value3);
		if (status == 0)
		{
			return 0;
		}
		if (*(float *)(mytype->min) <= value3 && value3 <= *(float *)(mytype->max))
		{
			*(float *)(mytype->data) = (float)value3;
			return 1;
		}
		return 0;
	case TYPE_DOUBLE:
		value3 = 0;
		status = (short)sscanf(data, "%lf", &value3);
		if (status == 0)
		{
			return 0;
		}
		if (*(double *)(mytype->min) <= value3 && value3 <= *(double *)(mytype->max))
		{
			*(double *)(mytype->data) = value3;
			return 1;
		}
		return 0;
		// case TYPE_CHAR_N:
		// case TYPE_STR_N:
	}
	return -1;
}

/*传入字符串和数据类型，将字符串内装换成对应的数据类型
注意：这个函数会申请内存，不用的时候记得释放
失败：return NULL
成功：return 申请到的储存转换结果的地址
malloc_len 请传入变量的地址，将会返回malloc申请到的长度

*/
void *get_value(char *data, int len_data, const enum type_id ID, unsigned char *malloc_len)
{
	short status = 0;
	long long value1;
	unsigned long long value2;
	double value3;
	void *end_data = NULL;
	switch (ID)
	{
	case TYPE_CHAR:
		//*(char *)(mytype->data) = data[0];
		*malloc_len = (unsigned char)sizeof(char);
		end_data = malloc(*malloc_len);
		if (end_data == NULL)
		{
			return NULL;
		}
		*(char *)end_data = data[0];
		return end_data;
	case TYPE_INT8:
		value1 = str_to_64(data, CANZHI_MAX_LEN, &status);
		if (status != 1)
		{
			return NULL;
		}
		*malloc_len = (unsigned char)sizeof(char);
		end_data = malloc(*malloc_len);
		if (end_data == NULL)
		{
			return NULL;
		}

		*(char *)end_data = (char)value1;
		// memccpy(end_data,data,sizeof(char));
		return end_data;
	case TYPE_u8:
		value1 = str_to_64(data, len_data, &status);
		if (status != 1)
		{
			return NULL;
		}
		*malloc_len = (unsigned char)sizeof(unsigned char);
		end_data = malloc(*malloc_len);
		if (end_data == NULL)
		{
			return NULL;
		}
		*(unsigned char *)(end_data) = (unsigned char)value1;
		return end_data;
	case TYPE_SHORT:
		value1 = str_to_64(data, len_data, &status);
		if (status != 1)
		{
			return NULL;
		}
		*malloc_len = (unsigned char)sizeof(short);
		end_data = malloc(*malloc_len);
		if (end_data == NULL)
		{
			return NULL;
		}
		*(short *)(end_data) = (short)value1;
		return end_data;
	case TYPE_USHORT:
		value1 = str_to_u16(data, len_data);
		if (value1 < 0)
		{
			return NULL;
		}
		*malloc_len = (unsigned char)sizeof(unsigned short);
		end_data = malloc(*malloc_len);
		if (end_data == NULL)
		{
			return NULL;
		}
		*(unsigned short *)(end_data) = (unsigned short)value1;
		return end_data;

	case TYPE_INT:
		value1 = str_to_64(data, len_data, &status);
		if (status != 1)
		{
			return NULL;
		}
		*malloc_len = (unsigned char)sizeof(int);
		end_data = malloc(*malloc_len);
		if (end_data == NULL)
		{
			return NULL;
		}
		*(int *)(end_data) = (int)value1;
		return end_data;

	case TYPE_UINT:
		value2 = str_to_u64(data, len_data, &status);
		if (status < 0)
		{
			return NULL;
		}
		*malloc_len = (unsigned char)sizeof(unsigned int);
		end_data = malloc(*malloc_len);
		if (end_data == NULL)
		{
			return NULL;
		}
		*(unsigned int *)(end_data) = (unsigned int)value2;
		return end_data;
	case TYPE_INT64:
		value1 = str_to_64(data, len_data, &status);
		if (status != 1)
		{
			return NULL;
		}
		*malloc_len = (unsigned char)sizeof(long long);
		end_data = malloc(*malloc_len);
		if (end_data == NULL)
		{
			return NULL;
		}
		*(long long *)(end_data) = value1;
		return end_data;
	case TYPE_U64:
		value2 = str_to_u64(data, len_data, &status);
		if (status < 0)
		{
			return NULL;
		}

		*malloc_len = (unsigned char)sizeof(unsigned long long);
		end_data = malloc(*malloc_len);
		if (end_data == NULL)
		{
			return NULL;
		}

		*(unsigned long long *)(end_data) = value2;
		return end_data;
	case TYPE_FLOAT:
		value3 = 0;
		status = (short)sscanf(data, "%lf", &value3);
		if (status == 0)
		{
			return NULL;
		}
		*malloc_len = (unsigned char)sizeof(float);
		end_data = malloc(*malloc_len);
		if (end_data == NULL)
		{
			return NULL;
		}
		*(float *)(end_data) = (float)value3;
		return end_data;
	case TYPE_DOUBLE:
		value3 = 0;
		status = (short)sscanf(data, "%lf", &value3);
		if (status == 0)
		{
			return NULL;
		}
		*malloc_len = (unsigned char)sizeof(double);
		end_data = malloc(*malloc_len);
		if (end_data == NULL)
		{
			return NULL;
		}
		*(double *)(end_data) = value3;
		return end_data;
		// case TYPE_CHAR_N:
		// case TYPE_STR_N:
	}
	return NULL;
}

#define UNEQUAL_FUHAO '~'
/*将数据拿来做比较
1 符合 data1  fuhao  data2
0 不符合，或者有错误


*/
int is_true(const enum type_id ID, void *data1, char fuhao, void *data2)
{
	switch (ID)
	{
	case TYPE_CHAR:
	case TYPE_INT8:
		switch (fuhao)
		{
		case '>':
			if (*(char *)data1 > *(char *)data2)
			{
				return 1;
			}
			return 0;
		case '<':
			if (*(char *)data1 < *(char *)data2)
			{
				return 1;
			}
			return 0;
		case '=':
			if (*(char *)data1 == *(char *)data2)
			{
				return 1;
			}
			return 0;
		case UNEQUAL_FUHAO:
			if (*(char *)data1 != *(char *)data2)
			{
				return 1;
			}
			return 0;
		}
	case TYPE_u8:
		switch (fuhao)
		{
		case '>':
			if (*(unsigned char *)data1 > *(unsigned char *)data2)
			{
				return 1;
			}
			return 0;
		case '<':
			if (*(unsigned char *)data1 < *(unsigned char *)data2)
			{
				return 1;
			}
			return 0;
		case '=':
			if (*(unsigned char *)data1 == *(unsigned char *)data2)
			{
				return 1;
			}
			return 0;
		case UNEQUAL_FUHAO:
			if (*(unsigned char *)data1 != *(unsigned char *)data2)
			{
				return 1;
			}
			return 0;
		}
	case TYPE_SHORT:
		switch (fuhao)
		{
		case '>':
			if (*(short *)data1 > *(short *)data2)
			{
				return 1;
			}
			return 0;
		case '<':
			if (*(short *)data1 < *(short *)data2)
			{
				return 1;
			}
			return 0;
		case '=':
			if (*(short *)data1 == *(short *)data2)
			{
				return 1;
			}
			return 0;
		case UNEQUAL_FUHAO:
			if (*(short *)data1 != *(short *)data2)
			{
				return 1;
			}
			return 0;
		}
	case TYPE_USHORT:
		switch (fuhao)
		{
		case '>':
			if (*(unsigned short *)data1 > *(unsigned short *)data2)
			{
				return 1;
			}
			return 0;
		case '<':
			if (*(unsigned short *)data1 < *(unsigned short *)data2)
			{
				return 1;
			}
			return 0;
		case '=':
			if (*(unsigned short *)data1 == *(unsigned short *)data2)
			{
				return 1;
			}
			return 0;
		case UNEQUAL_FUHAO:
			if (*(unsigned short *)data1 != *(unsigned short *)data2)
			{
				return 1;
			}
			return 0;
		}
	case TYPE_INT:
		switch (fuhao)
		{
		case '>':
			if (*(int *)data1 > *(int *)data2)
			{
				return 1;
			}
			return 0;
		case '<':
			if (*(int *)data1 < *(int *)data2)
			{
				return 1;
			}
			return 0;
		case '=':
			if (*(int *)data1 == *(int *)data2)
			{
				return 1;
			}
			return 0;
		case UNEQUAL_FUHAO:
			if (*(int *)data1 != *(int *)data2)
			{
				return 1;
			}
			return 0;
		}
	case TYPE_UINT:
		switch (fuhao)
		{
		case '>':
			if (*(unsigned int *)data1 > *(unsigned int *)data2)
			{
				return 1;
			}
			return 0;
		case '<':
			if (*(unsigned int *)data1 < *(unsigned int *)data2)
			{
				return 1;
			}
			return 0;
		case '=':
			if (*(unsigned int *)data1 == *(unsigned int *)data2)
			{
				return 1;
			}
			return 0;
		case UNEQUAL_FUHAO:
			if (*(unsigned int *)data1 != *(unsigned int *)data2)
			{
				return 1;
			}
			return 0;
		}
	case TYPE_INT64:
		switch (fuhao)
		{
		case '>':
			if (*(long long *)data1 > *(long long *)data2)
			{
				return 1;
			}
			return 0;
		case '<':
			if (*(long long *)data1 < *(long long *)data2)
			{
				return 1;
			}
			return 0;
		case '=':
			if (*(long long *)data1 == *(long long *)data2)
			{
				return 1;
			}
			return 0;
		case UNEQUAL_FUHAO:
			if (*(long long *)data1 != *(long long *)data2)
			{
				return 1;
			}
			return 0;
		}
	case TYPE_U64:
		switch (fuhao)
		{
		case '>':
			if (*(unsigned long long *)data1 > *(unsigned long long *)data2)
			{
				return 1;
			}
			return 0;
		case '<':
			if (*(unsigned long long *)data1 < *(unsigned long long *)data2)
			{
				return 1;
			}
			return 0;
		case '=':
			if (*(unsigned long long *)data1 == *(unsigned long long *)data2)
			{
				return 1;
			}
			return 0;
		case UNEQUAL_FUHAO:
			if (*(unsigned long long *)data1 != *(unsigned long long *)data2)
			{
				return 1;
			}
			return 0;
		}
	case TYPE_FLOAT:
		switch (fuhao)
		{
		case '>':
			if (*(float *)data1 > *(float *)data2)
			{
				return 1;
			}
			return 0;
		case '<':
			if (*(float *)data1 < *(float *)data2)
			{
				return 1;
			}
			return 0;
		case '=':
			if (*(float *)data1 == *(float *)data2)
			{
				return 1;
			}
			return 0;
		case UNEQUAL_FUHAO:
			if (*(float *)data1 != *(float *)data2)
			{
				return 1;
			}
			return 0;
		}
	case TYPE_DOUBLE:
		switch (fuhao)
		{
		case '>':
			if (*(double *)data1 > *(double *)data2)
			{
				return 1;
			}
			return 0;
		case '<':
			if (*(double *)data1 < *(double *)data2)
			{
				return 1;
			}
			return 0;
		case '=':
			if (*(double *)data1 == *(double *)data2)
			{
				return 1;
			}
			return 0;
		case UNEQUAL_FUHAO:
			if (*(double *)data1 != *(double *)data2)
			{
				return 1;
			}
			return 0;
		}
		// case TYPE_CHAR_N:
		// case TYPE_STR_N:
	}
	return 0;
}