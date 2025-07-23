/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "delay.h"
#include "arm_const_structs.h"
#include "ADC_u.h"
#include "TIM_u.h"
#include "Uart.h"
#include "SPI_u.h"
#include "AD9833.h"
#include "fft.h"
uint16_t f[2];
float32_t mag[2];
uint8_t type[2];

extern uint32_t ui32ADCClock;
extern uint32_t ui32ClockDiv;
extern int16_t adc_buff[FFT_LEN];
extern float32_t FFT_Mag[FFT_LEN];
extern uint32_t dataBuffer[FRE_MEASURE];
bool basic_flag=false;
bool ADC_Done=false;
bool ADC1Done=false;
volatile bool start_flag=false;
volatile bool lock_flag=true;
volatile bool SFP_now=0;
float32_t phase_arr[FRE_MEASURE];
/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	MAP_IntMasterEnable();
	TIM_TriggerADC_Init();
	ADC_SimpleInit();
	ADC1_SimpleInit();
//	ADC_Double_Init();
	UART_Init();//初始化UART0 与FPGA通信 协议需改
	VOFA_init();
	AD9833_SPI_Init();
	ADCClockConfigGet(ADC0_BASE,&ui32ClockDiv);
	ui32ADCClock=25e6/ui32ClockDiv;
	//字符串用""
	while(1)
	{
		//在URAT中修改
		while(start_flag)
		{
				uint16_t i;
				//ADC采样
				//todo 切换sequencer
				MAP_ADCSequenceEnable(ADC0_BASE, 2);//使能Sequence2 
				ADC_Change_freq(500e3);
				ADC_TimerStart();
				while(!ADC_Done);
				ADC_Done=false;
				MAP_ADCSequenceDisable(ADC0_BASE, 2);
				//shibie
				Myfft();
				
				shibie();//识别结果在f,type数组中
				//todo 切换sequencer
				
				//DDS输出分立信号 简单做题只实现一个DDS
				AD9833_WaveSeting(f[0],0,0,0);
				AD9833_Start(0,0,1-type[0]);
				SFP_now=0;
				//suopin
				MAP_ADCSequenceEnable(ADC1_BASE, 2);
				ADC_Change_freq(10);
				while(lock_flag){
					//读取ADC1的鉴频器数据 得到频率差
//					MAP_ADCProcessorTrigger(ADC1_BASE, (ADC_TRIGGER_WAIT | 3));
//					MAP_ADCProcessorTrigger(ADC0_BASE, (ADC_TRIGGER_SIGNAL | 3));
//					delay_us(60);
//					ADCSequenceDataGet(ADC0_BASE,3,&dataBufferA);
//					ADCSequenceDataGet(ADC1_BASE,3,&dataBufferB);
					//间隔时间过短，不采用
					
					ADC1_TimerStart();
					while(!ADC1Done);
					ADC1Done=false;
			
//					//计算相位差以计算频率  为了避免Hiebot变换 需要知道正弦的Vpp以提高转换速率
//					for(i=0;i<FRE_MEASURE;i++){
//						phase_arr[i]=dataBuffer
//					}
					//adc采样又需要偏置模块 ......
				}
				
				MAP_ADCSequenceDisable(ADC1_BASE, 2);
				
				
				
				
				
				
			
				
				
				
				
				
				
			
		}
	}
}


