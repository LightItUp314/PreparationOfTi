/* Includes ------------------------------------------------------------------*/
#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "system_init.h"
#include "delay.h"
#include "arm_const_structs.h"
#include "ADC_u.h"
#include "TIM_u.h"
#include "Uart.h"

#include "fft.h"
extern int16_t adc_buff[FFT_LEN];
extern float32_t FFT_Mag[FFT_LEN];
#define AM_TYPE 0x0
#define FM_TYPE 0x1
#define ASK_TYPE 0x2
#define FSK_TYPE 0x03
#define PSK_TYPE 0x04
float32_t mag_arr[15];
float32_t mag_max;
uint16_t mag_max_id;
uint16_t F;
uint16_t Rb;
float32_t h;
float32_t ma;
float32_t mf;
bool basic_flag=false;
bool ADC_Done=false;
float32_t parameter_b[64];
float32_t parameter_s[64];
/* Main ----------------------------------------------------------------------*/
int main(void)
{
	sys_init();
	MAP_IntMasterEnable();
	TIM_TriggerADC_Init();
	ADC_SimpleInit();
	UART_Init();//初始化UART0 与FPGA通信 协议需改
	VOFA_init();
	//初始化参数
	parameter_b[0]=140700;
	parameter_b[1]=0.10;
	parameter_s[0]=104342.105;
	parameter_s[1]=8;
	parameter_s[2]=125;
	parameter_s[3]=0.01;
	parameter_s[4]=0.0085;
	parameter_s[5]=12;
	parameter_s[6]=6;
	parameter_s[7]=10;
	parameter_s[8]=0.3;//maybe bigger than 0.3
	parameter_s[9]=5;
	parameter_s[10]=2;
	parameter_s[11]=0.02;
	parameter_s[12]=0.05;
	parameter_s[13]=0.2;
	parameter_s[14]=0.08;
	parameter_s[15]=5;
	//字符串用""
	while(1)
	{
		if(basic_flag){
			ADC_Change_freq(parameter_b[0]);
			ADC_TimerStart();
			while(!ADC_Done);
			ADC_Done=false;
			//FFT   是否需要低通滤波
			Myfft();//结果在FFT_Mag[FFT_LEN]
//			VOFA_SendData(adc_buff,FFT_Mag);
			uint8_t type;
			uint8_t i;
			uint8_t deltaId;
			type=shibie_basic(FFT_Mag,&mag_max,mag_arr);
			switch(type)
			{
				case 0:
				//FPGA,串口屏通信 CW
				
				break;
				
				
				case 1:
				
				for(i=0;i<5;i++)
				{
					if(mag_arr[i]>parameter_b[1]*mag_max)
					{
						F=(i+1)*1000;
						ma=2*mag_arr[i]/mag_max;
						break;
					}
				}
				//FPGA,串口屏通信 
				FPGA_SendType(AM_TYPE);
				break;
		 		case 2:
					VOFA_SendData(adc_buff,FFT_Mag);
					deltaId=find_min_index_diff_above_threshold(mag_arr,15,parameter_b[1]*mag_max);
					F=deltaId*1e3;
					guji_mf(F,&mf,parameter_s[7]);
					FPGA_SendType(FM_TYPE);
				break;
			}
			
			
			
		}else{
			//senior
			ADC_Change_freq(parameter_s[0]);
			ADC_TimerStart();
			while(!ADC_Done);
			ADC_Done=false;
			Myfft();
			VOFA_SendData(adc_buff,FFT_Mag);
//			VOFA_SendADC_Buf(adc_buff);
			if(shibie_2ask(adc_buff,parameter_s[1],parameter_s[2]))
			{
				
//				VOFA_SendData(adc_buff,FFT_Mag);
				
				guji_2ask_2(&Rb,parameter_s[7],parameter_s[8]);
				//通信
				FPGA_SendType(ASK_TYPE);
			}
			else{
				if(!shibie_2fskor2psk(parameter_s[7],parameter_s[8]))
				{
					//0 :2fsk
//					VOFA_SendData(adc_buff,FFT_Mag);
					guji_2fsk_2(&Rb,&h,parameter_s[7],parameter_s[8]);
					FPGA_SendType(FSK_TYPE);
					//通信
				}else{
					//1:psk
//					VOFA_SendData(adc_buff,FFT_Mag);
					guji_2psk_2(&Rb);
					//通信
					FPGA_SendType(PSK_TYPE);
				}
			}
		}
	}
}


