#ifndef AD9833_H
#define AD9833_H
#include "stdint.h"
#define FSS_1() GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4)
#define FSS_0() GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0)
#define TRI_WAVE 	0  		//输出三角波
#define SIN_WAVE 	1		//输出正弦波
#define SQU_WAVE 	2		//输出方波
void AD9833_Write(unsigned int data);
void AD9833_WaveSeting(double Freq,unsigned int Phase,unsigned int Freq_SFR,unsigned int Freq_SPR);
void AD9833_Start(uint8_t Freq_SFR, uint8_t Freq_SPR, uint8_t WaveMode);
void AD9833_SwitchFreqReg(uint8_t reg);
void AD9833_SwitchPhaseReg(uint8_t reg);
#endif