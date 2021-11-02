#include "mytimer.h"

const uint8_t month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
struct MyTime Now = {0, 1, 0, 1, 0, 0, 0};
//服务器每隔 1min 就会发来时间
void next_sec(struct MyTime *now)
{
	now->sec = now->sec + 1;
	if (now->sec >= 60)
	{
		now->sec = 0;
		now->minute = now->minute + 1;
		if (now->minute >= 60)
		{
			now->minute = 0;
			now->hour = now->hour + 1;
			if (now->hour >= 24)
			{
				now->hour = 0;
				now->day = now->day + 1;
				now->week = now->week + 1;
				//星期轮换
				if (now->week > 7)
				{
					now->week = 1;
				}
				//月份轮换
				if ((now->run_fig == 1 && now->month == 2 && now->day > 29) || (now->day > month_days[now->month - 1])) //二月计算闰
				{
					now->day = 1;
					now->month = now->month + 1;
					if (now->month > 12)
					{
						now->month = 1;
						now->year = now->year + 1;
						if (now->year % 100 == 0 && now->year % 400 == 0)
						{
							now->run_fig == 1;
						}
						else if (now->year % 100 > 0 && now->year % 4 == 0)
						{
							now->run_fig == 1;
						}
						else
						{
							now->run_fig == 0;
						}
					}
				}
			}
		}
	}
}
//将服务器发来的时间，转换
void str_get_time(struct MyTime *now, char *data_str)
{
	unsigned int tmp[7];
	sscanf(data_str, "Time%u-%u-%u %u %u:%u:%u", tmp, tmp + 1, tmp + 2, tmp + 3, tmp + 4, tmp + 5, tmp + 6);
	//&(now->year), &(now->month), &(now->day), &(now->week), &(now->hour), &(now->minute), &(now->sec)
	now->year = (uint16_t)tmp[0];
	now->month = (uint8_t)tmp[1];
	now->day = (uint8_t)tmp[2];
	now->week = (uint8_t)tmp[3];
	now->hour = (uint8_t)tmp[4];
	now->minute = (uint8_t)tmp[5];
	now->sec = (uint8_t)tmp[6];
	if (now->year % 100 == 0 && now->year % 400 == 0)
	{
		now->run_fig == 1;
	}
	else if (now->year % 100 > 0 && now->year % 4 == 0)
	{
		now->run_fig == 1;
	}
	else
	{
		now->run_fig == 0;
	}
}

/*
秒级定时器
time_s<26
2s 误差-1ms
*/
void set_timer1_s(timercallback userFunc, double time_s)
{
	timer1_isr_init();
	//timer1_enable(1, TIM_EDGE, TIM_LOOP);//分频，是否优先，是否重填
	//timer1_write(8000000);//count count<8388607
	//timer1 time = (16^分频)*count*0.0000000125  单位：s

	/*
	//2s 误差-1ms
	timer1_enable(2,TIM_EDGE,TIM_LOOP);
	timer1_write(312500*2);
	*/
	timer1_enable(2, TIM_EDGE, TIM_LOOP);
	timer1_write((uint32)(312500 * time_s));
	timer1_enabled();
	timer1_attachInterrupt(userFunc);
}

/*毫秒定时器 定时中断函数里禁止调用 delay 进行延时操作，调用必暴毙
time_ms < 1,677
userFunc 需要定时调用执行的函数名*/
void set_timer1_ms(timercallback userFunc, uint32_t time_ms)
{
	timer1_isr_init(); //系统函数，初始化定时器

	/* 
	//timer1_enable(1, TIM_EDGE, TIM_LOOP);//分频，是否优先，是否重填
	//timer1_write(8000000);//count count<8388607
	//timer1 time = (16^分频)*count*0.0000000125  单位：s  //默认esp8266时钟频率80MHz
	//1ms 误差~1ms ??? 定时100ms误差<1ms,定时10ms误差也是1ms，我也不知道为啥，
	//我没有示波器给我测试，但是我算出来的数据填充之后串口输出时间差就是这样，可能是串口耽误了时间吧
	timer1_enable(2,TIM_EDGE,TIM_LOOP);
	timer1_write(312500*2);
	*/
	timer1_enable(1, TIM_EDGE, TIM_LOOP);
	timer1_write((uint32)(5000 * time_ms));
	timer1_enabled();				  //使能中断
	timer1_attachInterrupt(userFunc); //填充
}

/*微秒定时器 定时中断函数里禁止调用 delay 进行延时操作，调用必暴毙
userFunc 需要定时调用执行的函数名*/
void set_timer1_us(timercallback userFunc, uint32_t time_us)
{
	timer1_isr_init(); //系统函数，初始化定时器

	/* 
	//timer1_enable(1, TIM_EDGE, TIM_LOOP);//分频，是否优先，是否重填
	//timer1_write(8000000);//count count<8388607
	//timer1 time = (16^分频)*count*0.0000000125  单位：s  //默认esp8266时钟频率80MHz
	//1ms 误差~1ms ??? 定时100ms误差<1ms,定时10ms误差也是1ms，我也不知道为啥，
	//我没有示波器给我测试，但是我算出来的数据填充之后串口输出时间差就是这样，可能是串口耽误了时间吧
	timer1_enable(2,TIM_EDGE,TIM_LOOP);
	timer1_write(312500*2);
	*/
	timer1_enable(1, TIM_EDGE, TIM_LOOP);
	timer1_write((uint32)(5 * time_us));
	timer1_enabled();				  //使能中断
	timer1_attachInterrupt(userFunc); //填充
}