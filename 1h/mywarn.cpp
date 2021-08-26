#include "mywarn.h"

extern "C"
{
	int len_warn = 0;
	struct WarnLink
	{
		struct Udpwarn *warn;
		struct WarnLink *next;
	};

	//关于此链表，会在警告状态为 NOT_WARN 时将链结删除，但是不会释放内存，因为有点地方是静态内存，不可以释放
	//为了方便管理，请管理好自己申请的内存，自己释放
	//可以调用 warn_exist 检查自己申请的内存是否处于被这里占用

	static struct WarnLink head = {NULL, NULL};
	static struct WarnLink *tail = &head;

//定义零时数组的长度，仅储存 "([mwe])(\d+)"
#define UDP_TMP_MAX 200

	//删掉p->next节点
	void del_next_link(struct WarnLink *p)
	{
		struct WarnLink *del; //储存要操作的节点
		del = p->next;
		p->next = p->next->next;
		if (tail == del)
		{
			tail = p; //尾巴节点向前移动一位
		}
		len_warn--;
		free(del);
	}

	/*检查 警告 是否已经在内存中出现了，根据警告的内存地址来判定警告是否相同。
	返回-1为没有占用，其他值为处在链结中的位置
	*/
	int warn_exist(struct Udpwarn *warn)
	{
		struct WarnLink *p = head.next;
		for (int i = 0; i < WARN_LEN; i++)
		{
			if (p == NULL)
			{
				return -1;
			}
			if (p->warn == warn)
			{
				return i;
			}
			p = p->next;
		}
		return -1;
	}

	int set_warn(struct Udpwarn *warn)
	{
		//Serial.printf(" set_warn %s   ", warn->str_waring);
		if (len_warn < WARN_LEN)
		{
			//记录错误
			if (warn_exist(warn) == -1)
			{
				tail->next = (WarnLink *)malloc(sizeof(WarnLink));
				if (tail->next == NULL)
				{
					Serial.printf(" set_warn error malloc NULL %d", len_warn);
				}
				//修改状态
				warn->status = IS_WARN;
				//修改触发时间
				warn->time = millis();
				//挂载错误到链结上
				tail->next->warn = warn;
				//赋值，用于判空
				tail->next->next = NULL;
				//记录错误总数
				len_warn++;
				//尾巴向后挪一位
				tail = tail->next;
				//Serial.printf(" set_warn len_warn %d", len_warn);
				//Serial.printf(" set_warn ok %d",len_warn);
			}
			//告知现在的错误总数
			return len_warn;
		}
		else
		{
			Serial.printf(" set_warn error len_warn %d", len_warn);
			return -1;
		}
	}

	void warn_send()
	{
		//遍历链表，对不同状态的错误做出反应
		struct WarnLink *p;	  //储存要操作的节点
		struct WarnLink *tmp; //储存上一个节点
		char udp_send_data[UDP_TMP_MAX] = {0};
		int data_len = 0;
		tmp = &head;
		p = head.next;
		if (p == NULL)
		{
			return;
		}
		//struct Udpwarn *warn;
		//Serial.printf(" warn_send in %d",len_warn);
		do
		{
			if (p->warn == NULL)
			{
				del_next_link(tmp);
				p = tmp->next;
				continue;
			}
			//Serial.printf(" do in status %d  ",p->warn->status);
			switch (p->warn->status)
			{
			case NOT_WARN:
				del_next_link(tmp);
				p = tmp;
				break;

			case TIMEOUT:
				//Serial.printf(" warn_send TIMEOUT ack timeout %u  %u %s  ",p->warn->id,p->warn->time,p->warn->str_waring);
				p->warn->time = millis(); //更新一下时间然后重新走发送流程，不更新时间的话，会导致高频发送
										  //Serial.printf(" timeout new %u   ",p->warn->time);

			case IS_WARN:
			case WAIT_SEND:
				//Serial.printf(" warn_send WAIT_SEND send %s   ",p->warn->str_waring);
				data_len = data_len + sprintf(udp_send_data + data_len, "%c%llu", (char)(p->warn->cmsg), p->warn->id);
				//UDP_send_data = UDP_send_data + String((char)(p->warn->cmsg)) + String(p->warn->id);
				p->warn->status = WAIT_ACK;
				//Serial.printf(" warn_send WAIT_SEND send end");
				break;

			case WAIT_ACK:
				if (millis() - p->warn->time > UDP_TIME_OUT_MS)
				{
					//Serial.printf(" warn_send WAIT_ACK timeout %s   ",p->warn->str_waring);
					p->warn->status = TIMEOUT; //重新发送
				}
				break;

			case WARN_ACK:
				break;
			}
			tmp = p;
			p = p->next;
		} while (p != NULL);
		if (data_len > 0)
		{
			Serial.printf(" UDP SNED DATA %s   ", udp_send_data);
			UDP_Send(MYHOST, UDP_PORT, UDP_head_data + String(udp_send_data));
		}
		//Serial.printf(" warn_send end  ");
		return;
	}

	int warn_ack(unsigned long long id, enum UdpMessageClass class1, char *tcp_send_data)
	{
		struct WarnLink *p;	  //储存要操作的节点
		struct WarnLink *tmp; //储存上一个节点
		int len = 1;
		*tcp_send_data = '#';
		tmp = &head;
		p = head.next;
		if (p == NULL)
		{
			return 0;
		}
		do
		{
			if (p->warn == NULL || p->warn->status == NOT_WARN)
			{
				del_next_link(tmp);
				p = tmp->next;
				continue;
			}
			if (p->warn->id == id && class1 == p->warn->cmsg)
			{
				p->warn->status = WARN_ACK;
				len = len + sprintf(tcp_send_data + len, "%s#", p->warn->str_waring);
				//strcpy(tcp_send_data,p->warn->str_waring);
				Serial.printf(" warn_ack ACK %llu  %s   ", id, p->warn->str_waring);
			}
			tmp = p;
			p = p->next;
		} while (p != NULL);
		return len;
	}
}