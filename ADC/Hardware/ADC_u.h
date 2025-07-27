#ifndef ADC_U_H
#define ADC_U_H
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "stdint.h"
#define HWREG(x)                                                              \
        (*((volatile uint32_t *)(x)))
#define TIMER_O_TAV             0x00000050  // GPTM Timer A Value
#define TIMER_O_TBV             0x00000054  // GPTM Timer B Value
#define ADC_O_PSSI              0x00000028  // ADC Processor Sample Sequence
#define ADC_TRIGGER_WAIT        0x08000000  // Wait for the synchronous trigger
#define ADC_TRIGGER_SIGNAL      0x80000000  // Signal the synchronous trigger
#define ADC_O_ISC               0x0000000C  // ADC Interrupt Status and Clear
#define ADC_O_ACTSS             0x00000000  // ADC Active Sample Sequencer
extern volatile bool ADC0_Done;
extern volatile bool ADC1_Done;
void ADC_SimpleInit(void);
void ADC_DualSimu_Init(void);
void ADC_DualSimu_TIM_Stop(void);
void ADC_DualSimu_TIM_Start(void);
/*
***********ADC结构说明************
Squencer是极其灵活的硬件设计，类似stm32中的注入组和规则组,但更加灵活！！！
，它用来管控ADC核心的采样顺序，逻辑（如采集哪个通道后触发中断，哪个后结束）
并且每个ADC都有多个Squencer可以设计不同的优先级，在不同的情况下指挥adc进行不同的采样，触发不同的中断
让MCU在不同情况下有不同的操作
*/
//AIN0 :PE3    AIN1:PE2    AIN2:PE1
#define FFT_LEN 10 
#endif