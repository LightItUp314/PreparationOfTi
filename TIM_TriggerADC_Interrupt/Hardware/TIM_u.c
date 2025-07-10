#include "TIM_u.h"
//使用tim0触发adc采样

extern uint32_t ui32SysClock;//在函数sys_init()中获取
//以tim0的timer_a或者2合一的32 bit timer为例，trigger adc
void TIM_TriggerADC_Init(void)
{
		//启动相应时钟
	  MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)));
		//配置时钟的mode和actTIMER_CFG_A_ACT_NONE：超时无动作（默认）  TIMER_CFG_A_ACT_TOGGLE：超时切换CCP引脚电平  TIMER_CFG_A_ACT_SETTO：超时设置CCP引脚为高。
		MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);//或者TIMER_CFG_PERIODIC 32位时TimerADCEventSet依然使用TIMER_ADC_TIMEOUT_A原因见MAP_TimerADCEventSet行
		//Sets the timer load value.  This function configures the timer load value; if the timer is running then the value is immediately loaded into the timer.
    //Only \b TIMER_A should be used when the timer is configured for full-width operation
	
		//也可以用分频器
		//Sets the timer prescale value. but!!!!!!! The prescaler is only operational when in half-width mode
		//MAP_TimerPrescaleSet(TIMER0_BASE, TIMER_A , 255);            value which must be between 0 and 255 (inclusive) for 16/32-bit timers.
	

	MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, (ui32SysClock)/10000);//1Hz 但是ui32SysClock超出了16位的范围 
		//ADC触发源的使能设置
    MAP_TimerADCEventSet(TIMER0_BASE, TIMER_ADC_TIMEOUT_A);//超时事件实际由TimerA的递减到0触发（硬件自动处理高低位联动），因此仍使用TIMER_ADC_TIMEOUT_A标志
		//Enables or disables the ADC trigger output.
    MAP_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
		
	/*
	TimerADCEventSet   选择哪些定时器事件可以触发ADC（如超时、捕获匹配等）。
	
	TimerControlTrigger	控制定时器是否向ADC模块输出触发信号（即是否允许事件实际触发ADC）。
			
	
	TimerADCEventSet 类似“选择闹钟触发条件”（如仅工作日响铃）。

	TimerControlTrigger 类似“打开闹钟开关”（即使条件满足，也需开关开启才会响铃）。
	*/
    MAP_TimerEnable(TIMER0_BASE, TIMER_A);
	
}

void TIM_Timeout_Interrup_Init(void)
{
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);	
    // Enable processor interrupts.
    //
    MAP_IntMasterEnable();

    //
    // Configure the two 32-bit periodic timers.
    //
    MAP_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, ui32SysClock);
    //
    // Setup the interrupts for the timer timeouts.
    //
    MAP_IntEnable(INT_TIMER1A);
    MAP_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Enable the timers.
    //
    MAP_TimerEnable(TIMER1_BASE, TIMER_A);
}

//ccp没写



void TIMER1A_IRQHandler(void)
{
	//这个函数直接清除指定的定时器中断标志位，
	//不需要先读取中断状态。这是因为定时器中断通常比较简单，
	//只有一个或少数几个中断源，因此可以直接清除。
	MAP_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	/*
	
	........
	
	*/
}

