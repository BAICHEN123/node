#include "savevalues.h"

struct Value_X
{
	void *data;
	int len;
};

struct Value_X; // 需要储存的变量值的特性，储存地址和字节长度
struct Value_X list_values[list_values_len_max];
int list_values_len = 0;

int get_list_values_len()
{
	return list_values_len;
}

int add_value(void *data, int len)
{
	list_values[list_values_len].len = len;
	list_values[list_values_len].data = data;
	list_values_len = list_values_len + 1;
	// printf("len=%d  ", len);
	return list_values_len;
}

/*配置写入文件*/
int save_values(const char *file_name)
{
	unsigned char *p_data; // 按字节调用数据的指针
	int write_statu;	   // 写入的状态
	int cou = 0;		   // 写入字节计数
	File dataFile = LittleFS.open(file_name, "w");
	for (int i = 0; i < list_values_len_max && i < list_values_len; i++)
	{
		p_data = (unsigned char *)list_values[i].data;
		for (int j = 0; j < list_values[i].len; j++)
		{
			write_statu = dataFile.write(p_data[j]);
			if (write_statu < 1)
			{
				dataFile.close();
				Serial.printf("write write_statu = %d \n", write_statu);
				return write_statu;
			}
			cou = cou + write_statu;
		}
	}
	dataFile.close(); // 完成文件写入后关闭文件
	Serial.printf("write count = %d \n", cou);
	return cou;
}

/*从文件里读取配置*/
int read_values(const char *file_name)
{
	unsigned char *p_data; // 按字节读取用的指针
	int read_data;		   // 读取一个字节的数据，判断是否为 -1（异常）
	int cou = 0;		   // 统计一共读取了多少字节
				 // 打开文件

	if (!LittleFS.exists(file_name))
	{
		return 0;
	}

	File dataFile = LittleFS.open(file_name, "r");

	for (int i = 0; i < list_values_len_max && i < list_values_len; i++)
	{
		p_data = (unsigned char *)list_values[i].data;
		for (int j = 0; j < list_values[i].len; j++)
		{
			read_data = dataFile.read();
			if (read_data == -1)
			{
				// printf("error");
				dataFile.close();
				Serial.printf("read read_data = %d \n", read_data);
				return -1;
			}
			p_data[j] = (unsigned char)read_data;
			cou = cou + 1;
			// scanf("%d-", &p_data[j]);
		}
		// scanf("\n", NULL);
	}

	// 再多读一个字节，看看是不是文件末尾，不然上一次操作减少字节之后，再次读取数据会错误，但是却不重启，不删除文件
	read_data = dataFile.read();
	if (read_data != -1)
	{
		return -1;
	}

	dataFile.close(); // 完成文件写入后关闭文件
	Serial.printf("read count = %d \n", cou);
	return cou;
}
// 删除文件
short file_delete(const char *file_name)
{
    if (LittleFS.begin())
    { // 启动SPIFFS
        Serial.println("SPIFFS Started.");
    }
    else
    {
        Serial.println("SPIFFS Failed to Start.");
        return -2; // 文件系统启动失败
    }

    // 确认闪存中file_name文件是否被清楚
    if (LittleFS.exists(file_name) && LittleFS.remove(file_name))
    {
        Serial.print(file_name);
        Serial.println(" found && delete");
    }
    else
    {
        Serial.print(file_name);
        Serial.print("NOT FOUND");
    }
    return 1;
}
