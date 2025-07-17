#include "arm_math.h"
#include "arm_const_structs.h"
#include "ADC_u.h"
#include "hamming.h"
#include "bessel.h"
#include "Uart.h"
float32_t FFT_In[FFT_LEN*2];
//float32_t	FFT_IN_Non[FFT_LEN*2];
float32_t FFT_Mag[FFT_LEN];
#define id_fc_sample 683
//688
#define delta_id 20
#define measure_len 81 
extern float32_t Fs;
extern float32_t mag_max;
extern int16_t adc_buff[FFT_LEN];
extern float32_t parameter_b[64];
extern float32_t parameter_s[64];
/* 宏：在 mag[index-Range .. index+Range] 内取最大值并存入 dst */
#define GET_MAX_AROUND(dst, mag, index, Range)            \
    do {                                                  \
        int16_t   _j;                                     \
        float32_t _max = 0;                               \
        for (_j = -(Range); _j <= (Range); ++_j)          \
            if (_max < (mag)[(index) + _j])               \
                _max = (mag)[(index) + _j];               \
        (dst) = _max;                                     \
    } while (0)
//序号和幅度值均改变

void Seek_Max0(float32_t arr[],uint16_t size,float32_t* temp,uint16_t *id_max)
{
	uint16_t i;
	*id_max=1;
	float32_t temp_max=arr[1];//pass直流分量
	for(i=1;i<size;i++)
	{
		if(arr[i]>temp_max)
		{
			temp_max=arr[i];
			*id_max=i;
		}
	}
	*temp=temp_max;
}
void Seek_Max1(float32_t arr[],uint16_t size,float32_t* temp)
{
	uint16_t i;
	float32_t temp_max=arr[1];//pass直流分量
	for(i=1;i<size;i++)
	{
		if(arr[i]>temp_max)
		{
			temp_max=arr[i];
		}
	}
	*temp=temp_max;
}
void Seek_Max2(float32_t arr[],uint16_t size,uint16_t * id_max)
{
	uint16_t i;
	*id_max=1;
	float32_t temp_max=arr[1];//pass数组第一个
	for(i=1;i<size;i++)
	{
		if(arr[i]>temp_max)
		{
			temp_max=arr[i];
			*id_max=i;
		}
	}
}

void Seek_Max(float32_t arr[],uint16_t id_mid,uint16_t range_half,float32_t *max)
{
	int16_t i;
	float32_t temp_max=arr[id_mid];
	for(i=-range_half;i<=range_half;i++)
	{
		if(arr[id_mid+i]>temp_max)
		{
			temp_max=arr[id_mid+i];
		}
	}
	*max=temp_max;
}

void Seek_Min(float32_t arr[],uint16_t id_mid,uint16_t range_half,float32_t *min)
{
	int16_t i;
	float32_t temp_min=arr[id_mid];
	for(i=-range_half;i<=range_half;i++)
	{
		if(arr[id_mid+i]<temp_min)
		{
			temp_min=arr[id_mid+i];
		}
	}
	*min=temp_min;
}
void Seek_Min1(float32_t arr[],uint16_t size,uint16_t * id_min)
{
	int16_t i;
	float32_t temp_min=arr[0];
	* id_min=0;
	for(i=0;i<=size;i++)
	{
		if(arr[i]<temp_min)
		{
			temp_min=arr[i];
			*id_min=i;
		}
	}
}
uint16_t binarySearchDescending(const float arr[], uint16_t size, float target) {
  // 边界检查
    if (target > arr[0]) return 0;       // target比最大值还大
    if (target <= arr[size - 1]) return size - 1;  // target ≤ 最小值  
		
		int left = 0;
    int right = size - 1;
    int result = -1;  // 默认-1，表示target比所有元素大

    while (left <= right) {
        int mid = left + (right - left) / 2;  // 防止溢出
        if (arr[mid] >= target) {
            result = mid;      // 记录候选位置
            left = mid + 1;    // 继续向右找更大的索引（因为数组是递减的）
        } else {
            right = mid - 1;   // 向左找
        }
    }

    
    return result;
}
//可能会用上
int isFlatUsingDSP(float numbers[], int size, float threshold) {
    float stdDev, mean;
    
    // 计算平均值
    arm_mean_f32(numbers, size, &mean);
    
    // 计算标准差
    arm_std_f32(numbers, size, &stdDev);
    
    // 如果标准差相对于平均值很小，则认为平坦
    return (stdDev / fabsf(mean)) <= threshold;
}
void movmean_f32(float32_t *src, float32_t *dst, uint32_t len, uint32_t win) {
    for (uint32_t i = 0; i < len; i++) {
        uint32_t start = (i < win / 2) ? 0 : i - win / 2;
        uint32_t end = (i + win / 2 >= len) ? len - 1 : i + win / 2;
        uint32_t count = end - start + 1;
        arm_mean_f32(&src[start], count, &dst[i]);
    }
}
uint8_t WhetherZeroDot(uint16_t id_mid,float32_t v_thred,uint8_t range_half,uint8_t num_thred)
{
	if(FFT_Mag[id_mid]<v_thred)
	{
		uint8_t i,num=0;
		for(i=0;i<range_half;i++)
		{
			if(FFT_Mag[id_mid+i]<=v_thred)
			{
				num++;
			}
		}
		for(i=0;i<range_half;i++)
		{
			if(FFT_Mag[id_mid-i]<=v_thred)
			{
				num++;
			}
		}
		if(num>num_thred)
		{
			return 1;
		}
		else{
			return 0;
		}
	}else{
		return 0;
	}
}
uint8_t WhetherZeroDot2(uint16_t id_mid,float32_t v_thred,uint8_t range_half)
{
	int16_t i;
	if(FFT_Mag[id_mid]<v_thred)
	{
		for(i=-range_half;i<=range_half;i++)
		{
			if(FFT_Mag[id_mid+i]>parameter_s[13])
			{
				return 0;
			}
		}
		float32_t temp;
		arm_mean_f32(&FFT_Mag[id_mid-range_half],2*range_half+1,&temp);
		if(temp<v_thred)
		{
			return 1;
		}else{
			return 0;
		}
	}else{
		return 0;
	}
}
void Myfft(bool basic_flag)
{

	//t:  (0:FFT_LEN-1)/fs
	uint16_t i;
	/*****************去平均(准确的说不是去直流)*/
	if(basic_flag){
		float32_t mean;
		uint32_t temp;
		temp = 0;
		for (i = 0; i < FFT_LEN; i++) {
				temp += adc_buff[i];
		}
		mean = (float32_t)temp / FFT_LEN;
		
		
		for(i=0;i<FFT_LEN;i++){
			FFT_In[2*i]=((float)adc_buff[i]-mean)*hamming_window[i]; //((float)adc_buff[i]-mean)*hamming_window[i]; 
			FFT_In[2*i+1]=0;
		}
		arm_cfft_f32(&arm_cfft_sR_f32_len4096,FFT_In ,0,1);//注意的是输入和输出共用一块缓存
		arm_cmplx_mag_f32(FFT_In, FFT_Mag,FFT_LEN);
	}else{
		for(i=0;i<FFT_LEN;i++){
			FFT_In[2*i]=((float)adc_buff[i])*hamming_window[i]; //((float)adc_buff[i]-mean)*hamming_window[i]; 
			FFT_In[2*i+1]=0;
		}
		arm_cfft_f32(&arm_cfft_sR_f32_len4096,FFT_In ,0,1);//注意的是输入和输出共用一块缓存
		arm_cmplx_mag_f32(FFT_In, FFT_Mag,FFT_LEN);
		FFT_Mag[0]=0;
		FFT_Mag[1]=0;
		FFT_Mag[2]=0;
		for(i=3;i<FFT_LEN/2;i++)
		{
			FFT_Mag[i]/=(FFT_LEN/2);
		}
	}
}

uint8_t shibie_basic(float *mag,float32_t*mag_max,float32_t mag_arr[])
{
	uint16_t i,id_mid=886;
	Seek_Max1(FFT_Mag,FFT_LEN/2,mag_max);
	uint16_t num=0,index=0;//30200左右为中心频率(函数设置问题导致偏差) id为  886
	for(i=1;i<=15;i++)
	{
		index=id_mid-(uint16_t)round(1000.0*i*FFT_LEN/Fs);
		GET_MAX_AROUND(mag_arr[i-1], mag, index, 2);
		if(mag_arr[i-1]>=(*mag_max)*parameter_b[1])
			{
				num++;
			}
	}
	if(num>2)
		{
			return 2;
		}else if(num==0){
			return 0;
		}else{
			return 1;
		}
}	
uint16_t find_min_index_diff_above_threshold(float32_t mag_arr[], uint32_t size, float32_t thred) {
    uint32_t min_diff = 0x0FFFFFFF;  // 初始化为最大整数值
    int prev_index = -1;     // 前一个大于阈值的元素索引
    
    for (uint32_t i = 0; i < size; i++) {
        if (mag_arr[i] > thred) {
            if (prev_index != -1) {  // 如果不是第一个大于阈值的元素
                int current_diff = i - prev_index;
                if (current_diff < min_diff) {
                    min_diff = current_diff;
                }
            }
            prev_index = i;  // 更新前一个有效索引
        }
    }
		if(min_diff == 0x0FFFFFFF){
//    printf_u(UART7_BASE,"find_min_index_diff_above_threshold error\n");
			return 0xFFFF;
		}else{
			return min_diff;  // 如果没有找到足够的元素，返回-1
		}
}
float32_t mag_fm_arr[4];//J0 J1 J2 J3 J4
void GetAccurateMf(uint8_t mf_id,uint8_t range_half,uint8_t * mf_id_accurate)
{
	int8_t i;
	float32_t temp_min=1e12,temp;
	for(i=-range_half;i<=range_half&&mf_id+i<measure_len;i++)
	{
		temp=powf(bessel[0][mf_id+i]-mag_fm_arr[0], 2)+powf(bessel[1][mf_id+i]-mag_fm_arr[1], 2)+powf(bessel[2][mf_id+i]-mag_fm_arr[2], 2)+powf(bessel[3][mf_id+i]-mag_fm_arr[3], 2);
		if(temp_min<temp)
		{
			*mf_id_accurate=mf_id+i;
		}
	}
}
void guji_mf(uint16_t F,float32_t* mf,float32_t range_half)
{
	uint8_t i,mf_id;
	for(i=0;i<4;i++)
	{
		Seek_Max(FFT_Mag,id_fc_sample-(int)(F*i*FFT_LEN/parameter_s[0]),range_half,&mag_fm_arr[i]);
	}
	float32_t measure=mag_fm_arr[2]/mag_fm_arr[3];
	mf_id=binarySearchDescending(measure_arr,measure_len,measure);
//	GetAccurateMf(mf_id,2,&mf_id);//目前没有归一化，函数不能用
	*mf=1+0.05f*mf_id;
}
bool shibie_2ask(int16_t adc_buff[],uint8_t range,float32_t thred)
{
	bool flag_temp=1;
	uint8_t j;
	uint16_t i;
	for(i=0;i<=FFT_LEN-range;i+=range)
	{
		for(j=1;j<range;j++)
		{
			if(adc_buff[i+j]-adc_buff[i]>thred||adc_buff[i+j]-adc_buff[i]<-thred)
			{
				flag_temp=0;
				break;
			}
		}
		if(flag_temp==1)
		{
			return true;
		}else{
			flag_temp=1;
		}
	}
	return false;
}
uint16_t id_r[5]={726,765,805,844,883};
//id_fc_sample 681 右侧 id:726   765   805   844   883
uint8_t whetherZero_arr[5];
uint8_t guji_2ask(void)
{
	
	uint8_t zeronums=0,i;
	whetherZero_arr[1]=WhetherZeroDot(id_r[1],parameter_s[3],parameter_s[4],parameter_s[5]);
	zeronums+=whetherZero_arr[1];
	for(i=2;i<5;i++)
	{
		whetherZero_arr[i]+=WhetherZeroDot(id_r[i],parameter_s[3],parameter_s[4],parameter_s[5]);
		zeronums+=whetherZero_arr[i];
	}
	if(zeronums>=4)
	{
		return 1;
	}
	for(i=1;i<5;i++)
	{
		if(whetherZero_arr[i]==1)
		{
			return (i+1);
		}		
	}
//	printf_u(UART7_BASE,"guji_2ask error non return\n");

	return 0;
}
//range_half: 30     thred:parameter_s[8]=0.3
bool shibie_2fskor2psk(float32_t range_half,float32_t thred)
{
	int16_t i;
	//	for(i=-range_half;i<=range_half;i++)
//	{
//		if(FFT_Mag[id_fc_sample+i]>thred)
//		{
//			return true;
//			break;
//		}
//	}
//	return false;
//	float32_t mean;
//	arm_mean_f32(&FFT_Mag[FFT_LEN-(uint8_t)(range_half/2)],range_half+1,&mean);
//	if(mean<)
//	uint16_t id_max;
	float32_t max_temp,max_temp1;
	uint16_t id_max;
	Seek_Max0(FFT_Mag,id_fc_sample-delta_id,&max_temp,&id_max);
	for(i=0;i<5;i++)
	{
		uint16_t id_f2sk_r_temp=round(0.5*(i+1)*1e3*FFT_LEN/parameter_s[0]);//010101.....
		if(id_max+id_f2sk_r_temp>id_fc_sample)
		{
			return 1;
		}
		Seek_Max(FFT_Mag,id_max+id_f2sk_r_temp,range_half,&max_temp1);
		if(max_temp1>thred*max_temp)
		{
			return 0;
		}
	}
	return 1;
}
float32_t min_f2sk_r[5];
void guji_2fsk(uint16_t* Rb,float32_t* h)
{
	uint16_t id_f1,i;
	Seek_Max2(&FFT_Mag[id_fc_sample],FFT_LEN-id_fc_sample+1,&id_f1);
	float32_t soomth_mag[FFT_LEN-id_fc_sample+1];
	movmean_f32(&FFT_Mag[id_fc_sample],soomth_mag,FFT_LEN-id_fc_sample,parameter_s[9]);//window_size
	for(i=0;i<5;i++)
	{
		uint16_t id_f2sk_r_temp=round(1.0*(i+1)*1e3*FFT_LEN/parameter_s[0]);
		Seek_Min(FFT_Mag,id_f1+id_f2sk_r_temp,parameter_s[10],&min_f2sk_r[i]);
	}
	uint8_t num=0;
	for(i=0;i<5;i++)
	{
		if(min_f2sk_r[i]<parameter_s[11])
		{
			num++;
		}
	}
	if(num>=4)
	{
		*Rb=1e3;
		*h=2*(id_f1*Fs/FFT_LEN-17500*1.0f)/1e3;
		return;
	}
	uint16_t id_min;
	Seek_Min1(&min_f2sk_r[1],FFT_LEN,&id_min);
	if(id_min==3-2||id_min==5-2)
	{
		*Rb=(i+2)*1e3;
		*h=2*(id_f1*Fs/FFT_LEN-17500*1.0f)/(*Rb);
		return;
	}else if(min_f2sk_r[1]<parameter_s[12]&&min_f2sk_r[3]<parameter_s[12])
	{
		*Rb=2e3;
		*h=2*(id_f1*Fs/FFT_LEN-17500*1.0f)/(*Rb);
		return;
	}else if(min_f2sk_r[1]>parameter_s[12]&&min_f2sk_r[3]<parameter_s[12])
	{
		*Rb=4e3;
		*h=2*(id_f1*Fs/FFT_LEN-17500*1.0f)/(*Rb);
		return;
	}
	*Rb=5e3;
	*h=2*(id_f1*Fs/FFT_LEN-17500*1.0f)/(*Rb);
}

void guji_2psk(uint16_t* Rb)
{
	uint8_t i;
	bool Zeros[5];
	for(i=0;i<5;i++)
	{
		Zeros[i]=(bool)WhetherZeroDot2(id_r[i],parameter_s[14],parameter_s[15]);
	}
	uint8_t zeronum=0;
	for(i=1;i<5;i++)
	{
		if(Zeros[i])
		{
			zeronum++;
		}
	}
	if(zeronum>=3)
	{
		* Rb=1e3;
		return;
	}
		for(i=1;i<4;i++)
	{
		if(Zeros[i])
		{
			* Rb=(i+1)*1e3;
		return;
		}
	}
}
void guji_2ask_2(uint16_t* Rb,float32_t range_half,float32_t thred)
{
	uint8_t i;
	uint16_t id_f0;
	float32_t max_temp,max_temp1;
	Seek_Max0(FFT_Mag,FFT_LEN/2,&max_temp,&id_f0);
//	Seek_Max0(FFT_Mag,id_f0-5,&max_temp,&id_f0);
	for(i=0;i<5;i++)
	{
		uint16_t id_f2sk_r_temp=round(1.0f*(i+1)*1e3*FFT_LEN/parameter_s[0]);//010101.....
		
		Seek_Max(FFT_Mag,id_f0+id_f2sk_r_temp,range_half,&max_temp1);
		if(max_temp1>thred*max_temp)
		{
			*Rb=(i+1)*1e3;
			return;
		}
	}
}
void guji_2fsk_2(uint16_t* Rb,float32_t* h,float32_t range_half,float32_t thred)
{
	uint16_t id_f1;
	float32_t max_temp,max_temp1;
	uint8_t i;
	Seek_Max0(FFT_Mag,id_fc_sample-delta_id,&max_temp,&id_f1);
	for(i=0;i<5;i++)
	{
		uint16_t id_f2sk_r_temp=round((i+1)*1e3*FFT_LEN/parameter_s[0]);//010101.....
		
		Seek_Max(FFT_Mag,id_f1+id_f2sk_r_temp,range_half,&max_temp1);
		if(max_temp1>thred*max_temp)
		{
			*Rb=(i+1)*1e3;
			*h=2*(id_fc_sample-id_f1)*Fs/FFT_LEN/(*Rb);
			return;
		}
	}
}
void guji_2psk_2(uint16_t* Rb)
{
	uint16_t id_max,temp;
	Seek_Max2(FFT_Mag,id_fc_sample,&id_max);
	temp=(uint16_t)((id_fc_sample-id_max)*Fs/FFT_LEN);
	* Rb=round(1.0f*temp/1000)*1000;
}