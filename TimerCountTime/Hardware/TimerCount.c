#include "TimerCount.h"
#include "math.h"
extern uint32_t ui32SysClock;
volatile uint32_t Atimes=0;
bool fre_measure_done=false;
volatile uint8_t arr_index;
time_measure EdgeTime[MEASURE_TIME];
//两个的MAP_TimerPrescaleSet();似乎没有效果！！！！！！！！！！！
//针对ui32Base的timerA计数器修改频率
void ADC_Change_freq(uint32_t ui32Base,float freq)
{
    // 检查无效频率
    if (freq <= 0)
    {
        return;
    }
    uint16_t psc;
    uint16_t period;

    // 计算目标值：clock / freq = (psc + 1) * (period + 1)
    uint32_t target = round(1.0f*ui32SysClock / freq);

    // 遍历 psc，寻找合适的 period
    for (psc = 0; psc < 65535; psc++)
    {
        // 如果除数超过目标值，跳过本次循环
        if (psc+1 > target)
        {
            return;
        }
        // 计算 period + 1 = target / divisor
        uint32_t period_plus_one = target/(psc+1);
        // 检查 period 是否在 16 位范围内 (最大 65535)
        if (period_plus_one > 65536) // period_plus_one <= 65536 确保 period <= 65535
        {
            continue;
        }
        // 验证是否精确匹配
        uint32_t actual = (psc+1)*period_plus_one;
        if (actual == target)
        {
            period = period_plus_one - 1;

            // 配置 TIM3
            MAP_TimerEnable(ui32Base, TIMER_A);                          // 关闭 TIM3 以进行配置
            MAP_TimerPrescaleSet(ui32Base, TIMER_A,psc);                 // 设置自动重载值 (period)
            TimerLoadSet(ui32Base,TIMER_A,period); 											// 设置预分频器
            return;
        }
    }
    // 未找到合适的 psc 和 period，可添加错误处理
    // 示例：printf("错误：无法为频率 %u 配置合适的 psc 和 period\n", freq);
}



void TimerStartTogether(void)
{
	//其实并非同步
	
	MAP_TimerIntClear(TIMER2_BASE, TIMER_CFG_B_CAP_TIME_UP);
	MAP_IntEnable(INT_TIMER2B);

//	MAP_TimerEnable(TIMER1_BASE, TIMER_A);
//	MAP_TimerEnable(TIMER2_BASE, TIMER_B);
	//faster
	
//	(*(volatile uint32_t *)(((uint32_t)0x40031000) + 0x0000000C))|=0x01;  // 启动 TIMER1A (TAEN)
//	
//	(*(volatile uint32_t *)(((uint32_t)0x40032000) + 0x0000000C))|=0x100;  // 启动 TIMER1A (TAEN)
	*(volatile uint32_t *)0x4003200C |= 0x100;  // 启动 TIMER2B (TBEN)	
	*(volatile uint32_t *)0x4003100C |= 0x01;   // 启动 TIMER1A (TAEN)
		
}
void TIM_Timeout_Interrup_Init(void)
{
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);	
		while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)));
	
	
    // Enable processor interrupts.
    //
    MAP_IntMasterEnable();
    //
    // Configure the two 32-bit periodic timers.
    //
    MAP_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC_UP);
    //设置周期时间  周期时间拉满了
//		MAP_TimerPrescaleSet(TIMER1_BASE,TIMER_A,0xff);
		MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, 0xffff);
	
		
    //
    // Setup the interrupts for the timer timeouts.
    //
    MAP_IntEnable(INT_TIMER1A);
    MAP_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Enable the timers.
    //
//    MAP_TimerEnable(TIMER1_BASE, TIMER_A);
}
void TIMER1A_IRQHandler(void)
{
	
	//这个函数直接清除指定的定时器中断标志位，
	//不需要先读取中断状态。这是因为定时器中断通常比较简单，
	//只有一个或少数几个中断源，因此可以直接清除。
	//MAP_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	//	uint32_t getTimerIntStatus;
	//	getTimerIntStatus=TimerIntStatus(TIMER2_BASE,true);
	//	if(!(getTimerIntStatus&TIMER_CAPB_EVENT)){
	//	Atimes++;
	//	}
	
	(*((volatile uint32_t *)(((uint32_t)0x40031000)+0x00000024)))=0x00000001;	
	if(!((*((volatile uint32_t *)(((uint32_t)0x40032000)+0x00000020)))& (1 << 10))){
	Atimes++;}
	
}
//PM1: T2CCP1    PM0: T2CCP0
void TIM_COM_Init()
{
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
  while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM)));
	MAP_GPIOPinConfigure(GPIO_PM1_T2CCP1);
	MAP_GPIOPinTypeTimer(GPIO_PORTM_BASE, GPIO_PIN_1);
//	GPIOM->PUR = GPIO_PIN_0;
}
void TIM_EdgeTime_Init(void)
{
		TIM_COM_Init();
		/* Enable the Timer-2 in 16-bit Edge Time mode */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2)));
    
		MAP_TimerConfigure(TIMER2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_TIME_UP);

    MAP_TimerControlEvent(TIMER2_BASE, TIMER_B, TIMER_EVENT_NEG_EDGE);
		//设置周期时间  周期时间拉满了
		//！！！！
//		MAP_TimerPrescaleSet(TIMER2_BASE,TIMER_B,0xff);
		MAP_TimerLoadSet(TIMER2_BASE, TIMER_B, 0xffff);
		MAP_TimerIntEnable(TIMER2_BASE, TIMER_CAPB_EVENT);

//  MAP_TimerEnable(TIMER2_BASE, TIMER_B);
	
		/* Enable the timer interrupt */
    MAP_IntEnable(INT_TIMER2B);
}
void TIMER2B_IRQHandler(void)
{
//		uint32_t getTimerIntStatus;

//    /* Get the timer interrupt status and clear the same */
//    getTimerIntStatus = MAP_TimerIntStatus(TIMER2_BASE, true);

//    MAP_TimerIntClear(TIMER2_BASE, getTimerIntStatus);
    //这个函数直接清除指定的定时器中断标志位，
		//不需要先读取中断状态。这是因为定时器中断通常比较简单，
		//只有一个或少数几个中断源，因此可以直接清除。
//    MAP_TimerIntClear(TIMER2_BASE, TIMER_CFG_B_CAP_TIME_UP);
		*(volatile uint32_t *)0x40032024 = 0x1700;
	
    EdgeTime[arr_index].times_h=Atimes;
		EdgeTime[arr_index++].times_l=MAP_TimerValueGet(TIMER2_BASE, TIMER_B);
		if(arr_index==MEASURE_TIME){
			MAP_IntDisable(INT_TIMER2B);
//		  MAP_TimerDisable(TIMER1_BASE, TIMER_A);
//			MAP_TimerDisable(TIMER2_BASE, TIMER_B);
			*(volatile uint32_t *)0x4003100C &= ~ 0x01;   // 关闭 TIMER1A (TAEN)
			*(volatile uint32_t *)0x4003200C &= ~ 0x100;  // 关闭 TIMER2B (TBEN)
			HWREG(TIMER1_BASE + 0x00000050)=0;
			HWREG(TIMER2_BASE + 0x00000054)=0;
			//将TV置为0 准备下次使用
			arr_index=0;
			fre_measure_done = 1;
		}
    
}
void TimerPrioritySet(void)
{
	//only looks at the upper 3 bits of the
	//! priority level
	IntPrioritySet(INT_TIMER2B,4<<5);
	IntPrioritySet(INT_TIMER1A,2<<5);
}


