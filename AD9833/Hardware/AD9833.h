#ifndef __AD9833_H
#define __AD9833_H	 
//#include "sys.h"
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "delay.h"
#define TRI_WAVE 	0  		//输出三角波
#define SIN_WAVE 	1		//输出正弦波
#define SQU_WAVE 	2		//输出方波
//使用k0 k1 k2 k3作为软件spi控制引脚
#define		FSYNC_1()     GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, GPIO_PIN_0)
#define		FSYNC_0()   	GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_0, 0)
#define   SCK_1()				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_1, GPIO_PIN_1)
#define 	SCK_0()				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_1, 0)
#define 	DAT_1()				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_2, GPIO_PIN_2)
#define 	DAT_0()				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_2, 0)
//#define 	CS_1()				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_3, GPIO_PIN_3);//高电平
//#define 	CS_0()				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_3, 0);//低电平
void AD9833_Init(void);
void AD9833_WaveSeting(double frequence,unsigned int frequence_SFR,unsigned int WaveMode,unsigned int Phase );
void AD9833_Init_GPIO(void);
void AD9833_AmpSet(unsigned char amp);		 				    
#endif