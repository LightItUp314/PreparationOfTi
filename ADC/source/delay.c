#include "delay.h"

void delay_us(uint16_t nus)//SysCtlDelay参数为1函数为3个时钟周期25ns
{
	SysCtlDelay(40* nus);//1ms
}

void delay_ms(uint16_t nms)
{
	SysCtlDelay(40000* nms);
}

