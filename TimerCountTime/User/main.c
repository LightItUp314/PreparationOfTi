/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "delay.h"
#include "TimerCount.h"
#define dev468750 2.133333333e-6
#define dev120e6 8.3333333333e-9
extern time_measure EdgeTime[MEASURE_TIME];
extern volatile uint32_t Atimes;
//delta count=delta count*256/120e6 
//0.1hz对应大概4e6需要22位数
double delta_time[MEASURE_TIME-1];


/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	TIM_Timeout_Interrup_Init();
	TIM_EdgeTime_Init();
	TimerPrioritySet();
	uint32_t i;
	while(1)
	{
		TimerStartTogether();
		while(!fre_measure_done);
		fre_measure_done=0;
		//分析
		uint32_t diff_h;
//		uint32_t diff_l;//低位相减是有可能是负数
		uint64_t total_diff;
		for(i=0;i<MEASURE_TIME-1;i++)
		{
			
				//delta_time[i]=((((EdgeTime[i+1].times_h-EdgeTime[i].times_h)<<16)+（EdgeTime[i+1].times_l-EdgeTime[i].times_l))/120e6;
			//优化得
			diff_h = EdgeTime[i+1].times_h - EdgeTime[i].times_h;
//			diff_l = EdgeTime[i+1].times_l - EdgeTime[i].times_l;
			total_diff = ((uint64_t)diff_h << 16) + EdgeTime[i+1].times_l - EdgeTime[i].times_l;
//			delta_time[i] = (double)total_diff/120e6;
			delta_time[i]=(double)total_diff*dev120e6;
		}
		Atimes=0;
		delay_ms(1000);
	}
}


