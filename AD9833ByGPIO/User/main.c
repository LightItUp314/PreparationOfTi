/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"

#include "AD9833ByGPIO.h"
/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	AD9833_Init();
	AD9833_WaveSeting(1e3,0,0,1);
	AD9833_Start(0,1,SQU_WAVE);
  while(1)
	{
	}
}


