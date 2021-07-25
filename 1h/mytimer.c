#include "mytimer.h"

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
