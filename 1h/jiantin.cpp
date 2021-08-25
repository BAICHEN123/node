#include "jiantin.h"
extern "C"
{

#define JianTin_MAX_LEN 5
	struct JianTin *link[JianTin_MAX_LEN];
	int link_len = 0;

	//将一个 struct JianTin 所指向的内存块全部释放，不释放本身
	void free_JianTin(struct JianTin *jt)
	{
		if (jt->name != NULL)
		{
			free(jt->name);
		}
		if (jt->data != NULL)
		{

			free(jt->data);
		}
	}

	int get_name_id(char *name, int data_len)
	{
		for (int i = 0; i < MAX_NAME; i++)
		{
			// int value = str1_find_char_1(my_tcp_cache.data, len_old, my_tcp_cache.len, '['); //获取 '[' 相对于 my_tcp_cache.data 的位置
			// if (value < 0 || value - len_old > 50)											 //限制名字的长度,找不到 '[' 就去找 ':'
			// {
			// 	Serial.printf("get '[' error value= %d %d\n", value, len_old);
			// 	value = str1_find_char_1(my_tcp_cache.data, len_old, my_tcp_cache.len, ':'); //获取':'相对于 my_tcp_cache.data 的位置
			// }
			// if (value < 0 || value - len_old > 50) //
			// {
			// 	Serial.printf("get ':' error value= %d \n", value);
			// 	break;
			// }
			if (1 == str1_eq_str2(name, 0, data_len, data_list[i].name))
			{
				return i;
			}
		}

		return -1;
	}
	/*
	
	return 	测试时返回0
			添加成功返回 0+
			-1 数据错误
			-2 内存申请失败
			-3 监听到达上限
	
	*/
	int add_jiantin(char *tcp_data, int data_len)
	{
		if (link_len >= JianTin_MAX_LEN)
		{
			return -3;
		}

		int name;
		int strd;
		struct JianTin jt = {0, -1, NULL, NULL, 0, 0, '?'};
		//int tmp = 1;
		short statu = 0;
		//tmp = sscanf(tcp_data, "A%llu#%50[^#]#%1[<>=]#%30s%c", &(jt.sql_id), strd[0], &(jt.fuhao), strd[1], &jt.fuhao);
		//解锁成就：发现一个电脑端gcc支持但是arduino支持不完善的函数


		jt.sql_id = str_to_u64(tcp_data, data_len, &statu);
		Serial.printf("sql_id %llu  %d\r\n", jt.sql_id,statu);
		if(statu!=1)
		{
			return -10;
		}

		name = str1_find_char_1(tcp_data, 1, data_len, '#'); //name前的#

		strd = str1_find_char_1(tcp_data, name + 1, data_len, '#'); //比较符号前的#

		if (name < 0 || strd < 0 || tcp_data[strd + 2] != '#')
		{
			return -11;
		}

		name = name + 1;
		tcp_data[strd] = '\0';
		jt.fuhao = tcp_data[strd + 1];
		strd = strd + 3;
		Serial.printf("tmp  %s  %s\r\n", tcp_data + name, tcp_data + strd);

		/*成功获取到数据，且数据格式没有问题

		name	strd[0]
		data	strd[1]
		其他的都存到 jt 里了
		*/
		//验证获取到的名字是否合法
		jt.name_len = strlen(tcp_data + name);
		if (jt.name_len < 1)
		{
			return -4;
		}

		//获取名字对应的id
		jt.name_id = get_name_id(tcp_data + name, jt.name_len);
		if (jt.name_id < 0)
		{
			Serial.printf("tmp  %s  %d\r\n", tcp_data + name, jt.name_len);
			return -5;
		}

		//验证获取到的参考数据是否合法
		jt.data_len = strlen(tcp_data + strd);
		if (jt.data_len < 1 || jt.data_len > 30)
		{
			return -6;
		}

		//将名字转存
		jt.name = (char *)malloc(jt.name_len);
		if (jt.name == NULL)
		{
			return -7;
		}
		strcpy(jt.name, tcp_data + name);

		//将字符串类型的数据转换成和数据条目相同的数据类型，然后储存
		jt.data = get_value(tcp_data + strd, jt.data_len, data_list[jt.name_id].ID, &jt.data_len);
		if (jt.data == NULL || jt.data_len == 0)
		{
			free_JianTin(&jt);
			return -8;
		}

		if(jt.sql_id==0llu)
		{
			//sql_id==0 的时候，是服务器在测试数据是否合法，到这里就可以了，后面都是数据存储
			return 0;
		}

		//验证所有的数据都合法之后，申请多块内存，储存前面的数据
		struct JianTin *jt1;

		jt1 = (struct JianTin *)malloc(sizeof(struct JianTin));
		if (jt1 == NULL)
		{
			free_JianTin(&jt);
			return -9;
		}

		//复制之前的数值，到申请号的内存里去
		memcpy(jt1, &jt, sizeof(struct JianTin));

		link[link_len] = jt1;
		link_len = link_len + 1;
		
		return jt.name_id+1;
	}

	void jiantin_print()
	{
		char tmp[50];
		for (int i = 0; i < link_len; i++)
		{
			get_data_str(link[i]->data, link[i]->data_len, data_list[link[i]->name_id].ID, tmp, link[i]->data_len);
			Serial.printf("jiantin_print  %d	%llu	%s %c %s\r\n", i, link[i]->sql_id, link[i]->name,link[i]->fuhao,tmp);

		}
	}
}