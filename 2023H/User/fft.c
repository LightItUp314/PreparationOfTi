#include "arm_math.h"
#include "arm_const_structs.h"
#include "ADC_u.h"
#include "hamming_1024.h"
#include "Uart.h"
float32_t FFT_In[FFT_LEN*2];
float32_t FFT_Mag[FFT_LEN];
float32_t FFT_Mag_Win[FFT_LEN];
extern int16_t adc_buff[FFT_LEN];
extern uint16_t f[2];
extern float32_t mag[2];
extern uint8_t type[2];
#define ADC_OFFSET (1650*4095/3300)    //1.65/3.3*4095
#define RANGE_HALF 4
#define THRED 800
			//*****************采样率500KHz******************
void Myfft(){
	//t:  (0:FFT_LEN-1)/fs
	uint16_t i;
	/*****************去平均(准确的说不是去直流)*/
	float32_t mean;
	uint32_t temp;
	temp = 0;
	for (i = 0; i < FFT_LEN; i++) {
			temp += adc_buff[i];
	}
	mean = (float32_t)temp / FFT_LEN;
	//不加窗
	for(i=0;i<FFT_LEN;i++){
			FFT_In[2*i]=((float)adc_buff[i]-mean); //((float)adc_buff[i]-mean)*hamming_window[i]; 
			FFT_In[2*i+1]=0;
		}
	arm_cfft_f32(&arm_cfft_sR_f32_len1024,FFT_In ,0,1);//注意的是输入和输出共用一块缓存
	arm_cmplx_mag_f32(FFT_In,FFT_Mag,FFT_LEN);
	FFT_Mag[0]=0;
	FFT_Mag[1]=0;
	FFT_Mag[2]=0;
	for(i=3;i<FFT_LEN/2;i++)
	{
		FFT_Mag[i]/=(FFT_LEN/2);
	}
	//加hamming
	for(i=0;i<FFT_LEN;i++){
		FFT_In[2*i]*=((float)adc_buff[i]-mean)*hamming_1024[i];
		FFT_In[2*i+1]=0;
	}
	arm_cfft_f32(&arm_cfft_sR_f32_len1024,FFT_In ,0,1);//注意的是输入和输出共用一块缓存
	arm_cmplx_mag_f32(FFT_In,FFT_Mag_Win,FFT_LEN);
	FFT_Mag_Win[0]=0;
	FFT_Mag_Win[1]=0;
	FFT_Mag_Win[2]=0;
}

void shibie(uint16_t* f,uint16_t* mag){
	uint16_t i,max_id_1=34,max_id_2=34;
	float32_t max_temp_1=FFT_Mag_Win[34],max_temp_2=FFT_Mag_Win[34];
	
	for(i=34;i<220;i++){
		if(FFT_Mag_Win[i]>max_temp_1){
			max_temp_1=FFT_Mag_Win[i];
			max_id_1=i;
		}
	}
	
	//寻找第二大
	for(i=34;i<220;i++){
		if(i<max_id_1+RANGE_HALF&&i>max_id_1-RANGE_HALF)
			continue;
		else if(FFT_Mag_Win[i]>max_temp_2){
			max_temp_2=FFT_Mag_Win[i];
			max_id_2=i;
		}
	}
	if(max_id_1<max_id_2){
		f[0]=round(max_id_1/1024*500e3/5e3)*5e3;
		mag[0]=max_temp_1;
		
		f[1]=round(max_id_2/1024*500e3/5e3)*5e3;
		mag[1]=max_temp_2;
	}else{
		f[1]=round(max_id_1/1024*500e3/5e3)*5e3;
		mag[1]=max_temp_1;
		
		f[0]=round(max_id_2/1024*500e3/5e3)*5e3;
		mag[0]=max_temp_2;
	}
	if(mag[0]>THRED){
		type[0]=0;
	}else{
		type[0]=1;
	}
	if(mag[1]>THRED){
		type[1]=0;
	}else{
		type[1]=1;
	}
}