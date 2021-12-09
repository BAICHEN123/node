#include "jiantin.h"
extern "C"
{
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

		//此监听不符合条件，消除警告，回收内存
		if (jt->warn != NULL)
		{
			//移除对 warn 的指向
			warn_del_warn(jt->warn);

			//释放 warm 指向的内存
			free((void *)(jt->warn->str_waring));
			//因为不同的位置，这里的字符串不一定是申请来的内存，所以不封装

			//释放 warm
			free(jt->warn);

			//清除指向
			jt->warn = NULL;
		}
	}

	/*删除一个监听的要求，释放内存，移动位置，
	释放 warn 指向的内存，释放 warn 的内存，释放只想 warn 的内存，释放 JianTin 内存
	*/
	void jiantin_del(unsigned long long sql_id)
	{
		int j;
		for (j = 0; j < link_len; j++)
		{
			if (link[j]->sql_id == sql_id)
			{
				//释放 jiantin 指向的内存
				free_JianTin(link[j]);
				//释放 jianting 的内存
				free(link[j]);
				//将最后一个监听移动过来，防止出现空缺//移除对 jiantin 的指向
				link_len = link_len - 1;
				link[j] = link[link_len];
				//设为空，方式被其他地方误读已经释放的位置
				link[link_len] = NULL;
				return;
			}
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

	/*查看此id是否已经被监听
	找到就返回索引号，找不到返回-1
	*/
	int jiantin_sql_id_exist(unsigned long long sql_id)
	{
		int i;
		for (i = 0; i < link_len; i++)
		{
			if (link[i]->sql_id == sql_id)
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
		//分析出来id之后在已经有的监听里查找一下，防止重复添加
		//id相同就不用新建监听了，把数据内容修改了就可以了，字符串内容需要修改
		Serial.printf("sql_id %llu  %d\r\n", jt.sql_id, statu);
		if (statu != 1)
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
		jt.name_len = strd - name; //计算内存长度获取长度值。之后申请内存前+1是因为要给字符串留一个字节存'\0'，这里不+1是因为名字比对的时候不算'\0'
		jt.fuhao = tcp_data[strd + 1];
		strd = strd + 3;
		Serial.printf("tmp  %s  %s\r\n", tcp_data + name, tcp_data + strd);

		/*成功获取到数据，且数据格式没有问题

		name	strd[0]
		data	strd[1]
		其他的都存到 jt 里了
		*/
		//验证获取到的名字是否合法
		//jt.name_len = strlen(tcp_data + name);
		if (jt.name_len < 2) //除去'\0'外至少有一个字节的数据
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
		jt.name_len = jt.name_len + 1;//多一个字节储存'\0'
		jt.name = (char *)malloc(jt.name_len);
		if (jt.name == NULL)
		{
			return -7;
		}
		strcpy(jt.name, tcp_data + name); //会复制'\0'

		//将字符串类型的数据转换成和数据条目相同的数据类型，然后储存
		jt.data = get_value(tcp_data + strd, jt.data_len, data_list[jt.name_id].ID, &jt.data_len);
		if (jt.data == NULL || jt.data_len == 0)
		{
			free_JianTin(&jt);
			return -8;
		}

		if (jt.sql_id == 0llu)
		{
			//sql_id==0 的时候，是服务器在测试数据是否合法，到这里就可以了，后面都是数据存储
			return 0;
		}

		//验证所有的数据都合法之后，申请多块内存，储存前面的数据
		struct JianTin *jt1;
		//查看id是否已经存在，已经存在的话，就不用新申请内存了，将之前的数据替换掉就可以了
		//name 里的值在之后用不到了，我拿来临时存储
		name = jiantin_sql_id_exist(jt.sql_id);
		if (name >= 0)
		{
			//id已经存在，修改即可。
			//先释放掉原来的储存名字和数据的内存
			free_JianTin(link[name]);
			jt1 = link[name];
		}
		else
		{
			//不存在，需要创建
			jt1 = (struct JianTin *)malloc(sizeof(struct JianTin));
			if (jt1 == NULL)
			{
				free_JianTin(&jt);
				return -9;
			}
			link[link_len] = jt1;
			link_len = link_len + 1;
		}

		//复制之前的数值，到申请号的内存里去
		jt.warn = NULL; //赋初值
		memcpy(jt1, &jt, sizeof(struct JianTin));

		return jt.name_id + 1;
	}

	void jiantin_print()
	{
		char tmp[50];
		for (int i = 0; i < link_len; i++)
		{
			get_data_str(link[i]->data, link[i]->data_len, data_list[link[i]->name_id].ID, tmp, link[i]->data_len);
			Serial.printf("jiantin_print  %d	%llu	%s %c %s\r\n", i, link[i]->sql_id, link[i]->name, link[i]->fuhao, tmp);
		}
	}

	int jiantin_loop()
	{
		int end = 0;
		char tmp[30];
		int str_len;
		char *tmp2;
		for (int i = 0; i < link_len; i++)
		{
			if (1 == is_true(data_list[link[i]->name_id].ID, data_list[link[i]->name_id].data, link[i]->fuhao, link[i]->data))
			{
				//符合此监听项的条件，需要向服务发送警告
				//检查是否已经有警告触发
				if (link[i]->warn == NULL)
				{
					link[i]->warn = (struct Udpwarn *)malloc(sizeof(struct Udpwarn));
					if (link[i]->warn == NULL)
					{
						end = end - 1;
						continue;
					}
					str_len = sprintf(tmp, "%llu", link[i]->sql_id);
					tmp2 = (char *)malloc(str_len + 1);
					if (tmp2 == NULL)
					{
						free(link[i]->warn);
						link[i]->warn = NULL;
						end = end - 1;
						continue;
					}

					link[i]->warn->id = link[i]->sql_id;
					link[i]->warn->cmsg = MESSAGE;
					strcpy(tmp2, tmp);	   //电脑上复制'\0'了，希望这里也复制了吧?
					*(tmp2 + str_len) = 0; //末尾设置成0,手动 0
					link[i]->warn->str_waring = tmp2;
					set_warn(link[i]->warn);
				}
				else if (link[i]->warn->status == NOT_WARN)
				{
					set_warn(link[i]->warn);
				}
			}
			else
			{
				//短时间内将警告设置为没有警告，超出一定时间之后将警告清除
				//减少相同事件在短时间内频繁触发
				if (link[i]->warn == NULL)
				{
					continue;
				}
				if (link[i]->warn->status != NOT_WARN)
				{
					warn_del_warn(link[i]->warn);
					link[i]->warn->status = NOT_WARN;
					link[i]->warn->time = millis(); //记录一个时间戳，超过多长时间都没有触发之后再回收内存
				}
				else if (millis() - link[i]->warn->time > WARN_DELETE_OUTTIME_MS)
				{
					//到达指定时间，回收内存

					//移除对 warn 的指向
					warn_del_warn(link[i]->warn);

					//释放 warm 指向的内存
					free((void *)(link[i]->warn->str_waring));
					//因为不同的位置，这里的字符串不一定是申请来的内存，所以不封装

					//释放 warm
					free(link[i]->warn);

					//清除指向
					link[i]->warn = NULL;
				}
			}
		}
		return end;
	}

	void set_not_warn(unsigned long long sql_id)
	{
		for (int i = 0; i < link_len; i++)
		{
			if (link[i]->sql_id == sql_id)
			{
				if (link[i]->warn == NULL)
				{
					return;
				}
				warn_del_warn(link[i]->warn);
				link[i]->warn->status = NOT_WARN;
				link[i]->warn->time = millis(); //记录一个时间戳，超过多长时间都没有触发之后再回收内存
				return;
			}
		}
	}
}