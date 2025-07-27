/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "TIM_u.h"
#include "ADC_u.h"

/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	//请在时钟配置结束后添加用户初始化代码
	TIM_TriggerADC_Init();
	ADC_DualSimu_Init();
	MAP_IntMasterEnable();
	//字符串用""
	while(1)
	{
		ADC_DualSimu_TIM_Start();
		while(!(ADC0_Done&&ADC1_Done));
		ADC0_Done=0;
		ADC1_Done=0;
	}
}


