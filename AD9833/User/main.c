/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "AD9833.h"

/* Main ----------------------------------------------------------------------*/
//AD9833输入电源电压2.5-5.5v
int main(void)
{
	sys_init();
	AD9833_Init();		  	//初始化与LED连接的硬件接口
	AD9833_WaveSeting(15000.0,0,SIN_WAVE,0 );//2KHz,	频率寄存器0，正弦波输出 ,初相位0 
  //AD9833_AmpSet(10);	//设置幅值，幅值最大 255
	FSYNC_1();
	//字符串用""
	while(1)
	{
	}
}


