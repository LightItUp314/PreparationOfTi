#ifndef __AD9833_H
#define __AD9833_H	 
//#include "sys.h"
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "delay.h"
typedef enum {
    SIN_WAVE= 0,   // Output triangle wave
    SQU_WAVE = 1,   // Output sine wave
    TRI_WAVE = 2    // Output square wave
} WaveMode;
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
void AD9833_WaveSeting(double Freq,unsigned int Phase,unsigned int Freq_SFR,unsigned int Freq_SPR);
void AD9833_Start(uint8_t Freq_SFR, uint8_t Freq_SPR, uint8_t WaveMode);
void AD9833_Init_GPIO(void);
void AD9833_AmpSet(unsigned char amp);		 				    
#endif