#include "system_init.h"

uint32_t ui32SysClock;

// SysTick_Handler interrupt
void SysTick_Handler(void)
{   
	
}

void sys_init()
{
	ui32SysClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480, 120000000);
	SysTickPeriodSet(ui32SysClock / 2); 
	SysTickIntRegister(SysTick_Handler);
	SysTickIntEnable();
	SysTickEnable();
}

