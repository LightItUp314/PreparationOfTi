#ifndef ADC_U_H
#define ADC_U_H
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "stdint.h"

/*
***********ADC结构说明************
Squencer是极其灵活的硬件设计，类似stm32中的注入组和规则组,但更加灵活！！！
，它用来管控ADC核心的采样顺序，逻辑（如采集哪个通道后触发中断，哪个后结束）
并且每个ADC都有多个Squencer可以设计不同的优先级，在不同的情况下指挥adc进行不同的采样，触发不同的中断
让MCU在不同情况下有不同的操作
*/
#define FFT_LEN 1024
#define FRE_MEASURE 4
#define ADC_dual_LEN 10
#define VPP_PHASE 2.42
void ADC_SimpleInit(void);
void ADC1_SimpleInit(void);
void ADC_TimerStart(void);
void ADC1_TimerStart(void);
void ADC_Change_freq(float freq);
#endif