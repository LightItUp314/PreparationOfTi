#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "GPIO_u.h"
#include "delay.h"
void AD9834_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2);
	FSYNC_1();
	SCK_0();
	DAT_0();
}
