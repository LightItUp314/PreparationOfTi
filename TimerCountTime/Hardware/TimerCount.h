#ifndef TIMER_COUNT_H
#define TIMER_COUNT_H
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#define MEASURE_TIME 10
extern bool fre_measure_done;

typedef struct{
	uint32_t times_h;
	uint32_t times_l;
}time_measure;
extern time_measure EdgeTime[MEASURE_TIME];
void ADC_Change_freq(uint32_t ui32Base,float freq);
void TimerStartTogether(void);
void TIM_Timeout_Interrup_Init(void);
void TIM_EdgeTime_Init(void);
void TimerPrioritySet(void);
#endif