#include "SPI_u.h"
#include "AD9833.h"
#include "delay.h"
//PE5 SSI1XDAT1(MASTER:RX) PE4 SSI1XDAT0(MASTER:TX)  PB5  SSI1Clk      PB4   SSI1Fss
//是的，AD9833 支持在单个 FSYNC 低电平周期内连续传输多个 16 位数据，
//且无需在数据之间将 FSYNC 拉高。这是 AD9833 SPI 接口的关键特性之一，也是其与标准 SPI 设备的主要区别
void AD9833_Delay(void)
{
	delay_us(5);
}

void AD9833_Write(unsigned int data)
{
	while(SSIBusy(SSI1_BASE));
	MAP_SSIDataPut(SSI1_BASE,data);
}
/*
*********************************************************************************************************
*	函 数 名: AD9833_WaveSeting
*	功能说明: 向SPI总线发送16个bit数据
*	形    参: 1.Freq: 频率值, 0.1 hz - 12Mhz
			  2.Freq_SFR: 0 或 1
			  3.WaveMode: TRI_WAVE(三角波),SIN_WAVE(正弦波),SQU_WAVE(方波)
			  4.Phase : 波形的初相位
*	返 回 值: 无
*********************************************************************************************************
*/ 
/*FTW = (所需频率 * 2<<28) / 系统时钟频率*/
/*(3) 设置相位
相位计算公式：

Phase Word
=
Desired Phase (deg)*4095/360
 
*/
void AD9833_WaveSeting(double Freq,unsigned int Phase,unsigned int Freq_SFR,unsigned int Freq_SPR)
{
		int frequence_LSB,frequence_MSB,Phs_data;
		double   frequence_mid,frequence_DATA;
		long int frequence_hex;

		/*********************************计算频率的16进制值***********************************/
		frequence_mid=268435456/25;//适合25M晶振
		frequence_DATA=Freq;
		frequence_DATA=frequence_DATA/1000000;
		frequence_DATA=frequence_DATA*frequence_mid;

		frequence_hex=frequence_DATA;  //这个frequence_hex的值是32位的一个很大的数字，需要拆分成两个14位进行处理；
		frequence_LSB=frequence_hex; //frequence_hex低16位送给frequence_LSB
		frequence_LSB=frequence_LSB&0x3fff;//去除最高两位，16位数换去掉高位后变成了14位
		frequence_MSB=frequence_hex>>14; //frequence_hex高16位送给frequence_HSB
		frequence_MSB=frequence_MSB&0x3fff;//去除最高两位，16位数换去掉高位后变成了14位
		SPI_AD9833_FFS(FFS_START);
		AD9833_Write(0x0100); //复位AD9833,即RESET位为1
		AD9833_Write(0x2100); //选择数据一次写入，B28位和RESET位为1
		if(Freq_SPR==0)				  //把数据设置到设置频率寄存器0
		{
			Phs_data=0xC000|Phase;
	  }else if(Freq_SPR==1){
			Phs_data=0xD000|Phase;
		}
		//修改频率寄存器
		if(Freq_SFR==0)				  //把数据设置到设置频率寄存器0
		{
		 	frequence_LSB=frequence_LSB|0x4000;
		 	frequence_MSB=frequence_MSB|0x4000;
			 //使用频率寄存器0输出波形
			AD9833_Write(frequence_LSB); //L14，选择频率寄存器0的低14位数据输入
			AD9833_Write(frequence_MSB); //H14 频率寄存器的高14位数据输入
			AD9833_Write(Phs_data);
			//AD9833_Write(0x2000); /**设置FSELECT位为0，芯片进入工作状态,频率寄存器0输出波形**/
	  }else if(Freq_SFR==1){
			frequence_LSB=frequence_LSB|0x8000;
			frequence_MSB=frequence_MSB|0x8000;
			//使用频率寄存器1输出波形
			AD9833_Write(frequence_LSB); //L14，选择频率寄存器1的低14位输入
			AD9833_Write(frequence_MSB); //H14 频率寄存器1为
			AD9833_Write(Phs_data);
		}
		SPI_AD9833_FFS(FFS_STOP);

}
//RESET=0 仅在芯片因复位（RESET=1）停止时有效，可重新启动输出。
/**
  * @brief  启动AD9833输出波形
  * @param  Freq_SFR 频率寄存器选择：0=FREQ0，1=FREQ1
  * @param  Freq_SPR 相位寄存器选择：0=PHASE0，1=PHASE1
  * @param  WaveMode 波形模式：SIN_WAVE(正弦波)/TRI_WAVE(三角波)/SQU_WAVE(方波)
  * @note   需提前通过AD9833_Write()配置好对应寄存器的频率/相位值
  */
void AD9833_Start(uint8_t Freq_SFR, uint8_t Freq_SPR, uint8_t WaveMode)
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
		SPI_AD9833_FFS(FFS_START);
    AD9833_Write(control_word);
		SPI_AD9833_FFS(FFS_STOP);
}
/**
  * @brief  切换AD9833的频率寄存器（无中断输出）
  * @param  reg 目标寄存器：0=FREQ0，1=FREQ1
  * @note   需提前在目标寄存器中写入频率值
  */
void AD9833_SwitchFreqReg(uint8_t reg) {
    uint16_t control_word = 0x2000;  // 基础控制字：RESET=0, OPBITEN=0, 正弦波
    
    // 设置FSELECT位（D11）
    if (reg == 1) {
        control_word |= (1 << 11);  // FSELECT=1 (FREQ1)
    }
    // 若reg=0，FSELECT保持0 (FREQ0)
    
    AD9833_Write(control_word);  // 切换频率寄存器
}
/**
  * @brief  切换AD9833的相位寄存器（无中断输出）
  * @param  reg 目标寄存器：0=PHASE0，1=PHASE1
  * @note   需提前在目标寄存器中写入相位值
  */
void AD9833_SwitchPhaseReg(uint8_t reg) {
    uint16_t control_word = 0x2000;  // 基础控制字：RESET=0, OPBITEN=0, 正弦波
    
    // 设置PSELECT位（D10）
    if (reg == 1) {
        control_word |= (1 << 10);  // PSELECT=1 (PHASE1)
    }
    // 若reg=0，PSELECT保持0 (PHASE0)
    
    AD9833_Write(control_word);  // 切换相位寄存器
}