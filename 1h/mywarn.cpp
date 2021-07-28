#include "mywarn.h"

extern "C"
{
	int len_warn = 0;
	struct WarnLink
	{
		struct Udpwarn *warn;
		struct WarnLink *next;
	};
	static struct WarnLink head = {NULL, NULL};
	static struct WarnLink *tail = &head;

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

	int warn_exist(struct Udpwarn *warn)
	{
		struct WarnLink *p = head.next;
		for (int i = 0; i < len_warn; i++)
		{
			if (p == NULL)
			{
				return -1;
			}
			else if (p->warn = warn)
			{
				return i;
			}
			p = p->next;
		}
		return -1;
	}

	int set_warn(struct Udpwarn *warn)
	{
		Serial.printf(" set_warn %s   ", warn->str_waring);
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
				p->warn->time = millis();//更新一下时间然后重新走发送流程，不更新时间的话，会导致高频发送
				Serial.printf(" warn_send TIMEOUT ack timeout");

			case IS_WARN:
			case WAIT_SEND:
				//Serial.printf(" warn_send WAIT_SEND send %s   ",p->warn->str_waring);
				UDP_Send(MYHOST, UDP_PORT, UDP_head_data + String((char)(p->warn->cmsg)) + String(p->warn->id) + "," + String(p->warn->str_waring));
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
		//Serial.printf(" warn_send end  ");
		return;
	}

	void warn_ack(unsigned int id)
	{
		struct WarnLink *p;	  //储存要操作的节点
		struct WarnLink *tmp; //储存上一个节点
		tmp = &head;
		p = head.next;
		if (p == NULL)
		{
			return;
		}
		do
		{
			if (p->warn == NULL || p->warn->status == NOT_WARN)
			{
				del_next_link(tmp);
				p = tmp->next;
				continue;
			}
			if (p->warn->id == id)
			{
				p->warn->status = WARN_ACK;
				Serial.printf(" warn_ack ACK %d  %s   ", id, p->warn->str_waring);
			}
			tmp = p;
			p = p->next;
		} while (p != NULL);
	}
}