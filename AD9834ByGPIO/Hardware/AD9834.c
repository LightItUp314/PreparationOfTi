#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include "AD9834.h"
#include "GPIO_u.h"
#include "delay.h"
/*
*********************************************************************************************************
*	函 数 名: AD9834_Delay
*	功能说明: 时钟延时
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AD9834_Delay(void)
{
	delay_us(50);
}
/*
*********************************************************************************************************
*	函 数 名: AD9834_Write
*	功能说明: 向SPI总线发送16个bit数据
*	形    参: TxData : 数据
*	返 回 值: 无
*********************************************************************************************************
*/
void AD9834_Write(unsigned int TxData)
{
	unsigned char i;

	SCK_1();
	AD9834_Delay();
	FSYNC_0();
	AD9834_Delay();
	for(i = 0; i < 16; i++)
	{
		if(TxData&0x8000){
			DAT_1();
			//由于msp432主频高加入延时
		}
		else{
			DAT_0();
		}
		AD9834_Delay();		
		SCK_0();
		AD9834_Delay();		
		SCK_1();
		AD9834_Delay();
		TxData <<= 1;
	}
	FSYNC_1();
	AD9834_Delay();//等待抬高
	SCK_0();
}

/*
*********************************************************************************************************
*	函 数 名: AD9834_AmpSet
*	功能说明: 改变输出信号幅度值(需要有程控电位器的AD9834)
*	形    参: 1.amp ：幅度值  0- 255
*	返 回 值: 无
*********************************************************************************************************
*/ 


void AD9834_AmpSet(unsigned char amp)
{
	unsigned char i;
	unsigned int temp;
  SCK_1();
	AD9834_Delay();
	FSYNC_0();
	AD9834_Delay();
	temp =0x1100|amp;
	for(i=0;i<16;i++)
	{	
	   if(temp&0x8000){
	   	DAT_1();
		 }
	   else{
			DAT_0();
		 }
		temp<<=1;
	  AD9834_Delay();		
		SCK_0();
		AD9834_Delay();		
		SCK_1();
		AD9834_Delay();
	}
  FSYNC_1();
	AD9834_Delay();//等待抬高
	SCK_0();
}



/*
*********************************************************************************************************
*	函 数 名: AD9834_WaveSeting
*	功能说明: 向SPI总线发送16个bit数据
*	形    参: 1.Freq: 频率值, 0.1 hz - 12Mhz
			  2.Freq_SFR: 0 或 1
			  3.WaveMode: TRI_WAVE(三角波),SIN_WAVE(正弦波),SQU_WAVE(方波)
			  4.Phase : 波形的初相位
*	返 回 值: 无
*********************************************************************************************************
*/ 
void AD9834_WaveSeting(double Freq,unsigned int Phase,unsigned int Freq_SFR,unsigned int Freq_SPR)
{

		int frequence_LSB,frequence_MSB,Phs_data;
		double   frequence_mid;
		long int frequence_hex;

		/*********************************计算频率的16进制值***********************************/
		frequence_mid=268435456.0/AD9834_SYSTEM_COLCK*Freq;
		//如果时钟频率不为25MHZ，修改该处的频率值，单位MHz ，AD9834最大支持25MHz
		frequence_hex=frequence_mid;  //这个frequence_hex的值是32位的一个很大的数字，需要拆分成两个14位进行处理；
		frequence_LSB=frequence_hex; //frequence_hex低16位送给frequence_LSB
		frequence_LSB=frequence_LSB&0x3fff;//去除最高两位，16位数换去掉高位后变成了14位
		frequence_MSB=frequence_hex>>14; //frequence_hex高16位送给frequence_HSB
		frequence_MSB=frequence_MSB&0x3fff;//去除最高两位，16位数换去掉高位后变成了14位
		//选择相位修改的寄存器
		if(Freq_SPR==0)
			Phs_data=Phase|0xC000;	//相位值
		else{
			Phs_data=Phase|0xD000;	//相位值
		}
		AD9834_Write(0x0100); //复位AD9834,即RESET位为1
		AD9834_Write(0x2100); //选择数据一次写入，B28位和RESET位为1

		if(Freq_SFR==0)				  //把数据设置到设置频率寄存器0
		{
		 	frequence_LSB=frequence_LSB|0x4000;
		 	frequence_MSB=frequence_MSB|0x4000;
			 //使用频率寄存器0输出波形
			AD9834_Write(frequence_LSB); //L14，选择频率寄存器0的低14位数据输入
			AD9834_Write(frequence_MSB); //H14 频率寄存器的高14位数据输入
			AD9834_Write(Phs_data);	//设置相位
			//AD9834_Write(0x2000); /**设置FSELECT位为0，芯片进入工作状态,频率寄存器0输出波形**/
	    }
		if(Freq_SFR==1)				//把数据设置到设置频率寄存器1
		{
			 frequence_LSB=frequence_LSB|0x8000;
			 frequence_MSB=frequence_MSB|0x8000;
			//使用频率寄存器1输出波形
			AD9834_Write(frequence_LSB); //L14，选择频率寄存器1的低14位输入
			AD9834_Write(frequence_MSB); //H14 频率寄存器1为
			AD9834_Write(Phs_data);	//设置相位
			//AD9834_Write(0x2800); /**设置FSELECT位为0，设置FSELECT位为1，即使用频率寄存器1的值，芯片进入工作状态,频率寄存器1输出波形**/
		}

//		if(WaveMode==TRI_WAVE) //输出三角波波形
//		 	AD9834_Write(0x2002); 
//		if(WaveMode==SQU_WAVE)	//输出方波波形
//			AD9834_Write(0x2028); 
//		if(WaveMode==SIN_WAVE)	//输出正弦波形
//			AD9834_Write(0x2000); 
//		uint16_t control_word = 0x2000;  // 基础控制字：RESET=0, OPBITEN=0
//    
//    // 1. 设置频率寄存器选择位(FSELECT)
//    if(Freq_SFR == 1) {
//        control_word |= (1 << 11);  // FSELECT=1 (FREQ1)
//    }
//    
//    // 2. 设置相位寄存器选择位(PSELECT)
//    if(Freq_SPR == 1) {
//        control_word |= (1 << 10);  // PSELECT=1 (PHASE1)
//    }
//    
//    // 3. 设置波形模式
//    switch(WaveMode) {
//        case TRI_WAVE:  // 三角波
//            control_word |= (1 << 1);  // MODE=1
//            break;
//            
//        case SQU_WAVE:  // 方波
//            control_word |= (1 << 5);  // OPBITEN=1 (方波模式)
//            control_word |= (1 << 3);  // DIV2=1 (不分频)
//            break;
//            
//        case SIN_WAVE:  // 正弦波(默认)
//        default:
//            // control_word保持默认值0x2000
//            break;
//    }
//    
//    // 4. 发送控制字启动输出
//    AD9834_Write(control_word);


}
//RESET=0 仅在芯片因复位（RESET=1）停止时有效，可重新启动输出。
/**
  * @brief  启动AD9834输出波形
  * @param  Freq_SFR 频率寄存器选择：0=FREQ0，1=FREQ1
  * @param  Freq_SPR 相位寄存器选择：0=PHASE0，1=PHASE1
  * @param  WaveMode 波形模式：SIN_WAVE(正弦波)/TRI_WAVE(三角波)/SQU_WAVE(方波)
  * @note   需提前通过AD9834_Write()配置好对应寄存器的频率/相位值
  */
void AD9834_Start(uint8_t Freq_SFR, uint8_t Freq_SPR, uint8_t WaveMode)
{
    uint16_t control_word = 0x2000;  // 基础控制字：RESET=0, OPBITEN=0
    
    // 1. 设置频率寄存器选择位(FSELECT)
    if(Freq_SFR == 1) {
        control_word |= (1 << 11);  // FSELECT=1 (FREQ1)
    }
    
    // 2. 设置相位寄存器选择位(PSELECT)
    if(Freq_SPR == 1) {
        control_word |= (1 << 10);  // PSELECT=1 (PHASE1)
    }
    
    // 3. 设置波形模式
    switch(WaveMode) {
        case TRI_WAVE:  // 三角波
            control_word |= (1 << 1);  // MODE=1
            break;
            
        case SQU_WAVE:  // 方波
            control_word |= (1 << 5);  // OPBITEN=1 (方波模式)
            control_word |= (1 << 3);  // DIV2=1 (不分频)
            break;
            
        case SIN_WAVE:  // 正弦波(默认)
        default:
            // control_word保持默认值0x2000
            break;
    }
    
    // 4. 发送控制字启动输出
    AD9834_Write(control_word);
}