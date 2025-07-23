/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "GPIO_u.h"
#include "AD9834.h"
/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	AD9834_Init();
	AD9834_WaveSeting(3e3,0,0,0);
	AD9834_Start(0,0,SQU_WAVE);
	//字符串用""
	while(1)
	{
	}
}


