#ifndef GPIO_U_H
#define GPIO_U_H
//使用k0 k1 k2 k3作为软件spi控制引脚
#define		FSYNC_1()     GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, GPIO_PIN_0)
#define		FSYNC_0()   	GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, 0)
#define   SCK_1()				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_1, GPIO_PIN_1)
#define 	SCK_0()				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_1, 0)
#define 	DAT_1()				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_2, GPIO_PIN_2)
#define 	DAT_0()				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_2, 0)
void AD9834_Init(void);
#endif