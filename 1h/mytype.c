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

/*将 struct MyType mytype 内的 mytype.name 转换成char[]
填充到 char[] 里去
返回填充进去的长度
请自行确保 data 剩余空间至少50字节，因为我关于数字转换的地方都没有确认
*/
int get_name_str(struct MyType *mytype, char *data, int len_data)
{
	int len = 0;
	if (mytype->name[0] == '@')
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
			return sprintf(data, "%s[%.0f-%.0f]", mytype->name, *(float *)(mytype->min), *(float *)(mytype->max));
		case TYPE_DOUBLE:
			return sprintf(data, "%s[%.0f-%.0f]", mytype->name, *(double *)(mytype->min), *(double *)(mytype->max));
		//case TYPE_CHAR_N:
		//	return sprintf(data, "%s", (char *)(mytype->data));
		//case TYPE_STR_N:
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
	len = get_data_str(mytype, data, len_data);
	if (mytype->unit != NULL)
	{
		len = len + sprintf(data + len, "%s", mytype->unit);
	}
	return len;
}

/*
传入的data必须是字符串的开始位置，可以直接data[0]访问
*/

int set_value(struct MyType *mytype, char *data, int len_data)
{

	int status = 0;
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
			*(unsigned short *)(mytype->data) = (short)value1;
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
		if (status != 1)
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
		if (status != 1)
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
		status = sscanf(data, "%lf", &value3);
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
		status = sscanf(data, "%lf", &value3);
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
	//case TYPE_CHAR_N:
	//case TYPE_STR_N:
	default:
		return -1;
	}
}