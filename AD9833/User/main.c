/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "AD9833.h"
#include "SPI_u.h"
/* Main ----------------------------------------------------------------------*/
//AD9833输入电源电压2.5-5.5v
int main(void)
{
	sys_init();
	AD9833_SPI_Init();		  	//初始化与LED连接的硬件接口 
  //AD9833_AmpSet(10);	//设置幅值，幅值最大 255
	AD9833_WaveSeting(3000.0,0,0,0);
	 
	AD9833_Start(0,0,SIN_WAVE);
	//字符串用""
	while(1)
	{
		
	}
}


