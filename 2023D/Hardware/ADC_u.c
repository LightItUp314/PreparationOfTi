#include "ADC_u.h"
#include "TIM_u.h"
#include "arm_math.h"
#define TIMER_O_TAV             0x00000050  // GPTM Timer A Value
#define TIMER_O_TBV             0x00000054  // GPTM Timer B Value
/* The control table used by the uDMA controller.  This table must be aligned
 * to a 1024 byte boundary. */
 
 
#if defined(__ICCARM__)
#pragma data_alignment=1024
uint8_t pui8ControlTable[1024];
#elif defined(__TI_ARM__)
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];
#else
uint8_t pui8ControlTable[1024] __attribute__ ((aligned(1024)));
#endif

int16_t adc_buff[FFT_LEN];
float32_t Fs;
extern uint32_t ui32SysClock;
extern bool ADC_Done;

void ADC_SimpleInit(void)
{
	//modul init*****************************************
	/* Enable the clock to GPIO Port E and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)));

    /* Configure PE1 as ADC input channel */
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);

    /* Enable the clock to ADC-0 and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)));

		//Squencer Init**************************************
		/*设计Squencer中的certain step 
		Configure Sequencer 2 to sample the analog channel : AIN0. The
     * end of conversion and interrupt generation is set for AIN0 */
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH2 | 
                                 ADC_CTL_END);// 去掉ADC_CTL_IE orADC采样序列完成就会触发中断  虽然但是，用了ADC_CTL_IE好像没啥影响，但是去掉也能用
																							//  ADC_CTL_IE 和 ADC_INT_DMA_SS3，但它们的 中断服务程序是同一个（ADC0SS3_IRQHandler
																							//通常用于软件读取ADC数据 但我们用dma
    /* Enable sample sequence 2 with a Processor signal trigger.  Sequencer 2
     * will do a single sample.*/
		//关于时钟的配置见相关工程中timer0的配置 
    MAP_ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_TIMER, 2);//1个触发信号 → 执行Sequencer内所有Steps
		//Squencer大致配置完毕
		/* Since sample sequence 2 is now configured, it must be enabled. */
    MAP_ADCSequenceEnable(ADC0_BASE, 2);//使能Sequence
		//首先配置dma相关参数
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
		while(!(SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA)));

		MAP_uDMAEnable();
		MAP_uDMAControlBaseSet(pui8ControlTable);
		MAP_uDMAChannelAssign(UDMA_CH16_ADC0_2);//ADC0_2表示ADC0的Squencer 2

		MAP_uDMAChannelAttributeDisable(UDMA_CH16_ADC0_2,
																	UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
																	UDMA_ATTR_HIGH_PRIORITY |
																	UDMA_ATTR_REQMASK);

		MAP_uDMAChannelControlSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
														 UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 |
														 UDMA_ARB_4);

		MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
															UDMA_MODE_BASIC,
															(void *)&ADC0->SSFIFO2, (void *)&adc_buff,
															sizeof(adc_buff)/2);
		
		// 2. 配置ADC序列器和中断
		//dma涉及中断，需要配置中断
		/* Clear the interrupt status flag before enabling. This is done to make
     * sure the interrupt flag is cleared before we sample. */
		MAP_ADCIntClearEx(ADC0_BASE, ADC_INT_DMA_SS2);
    MAP_ADCIntEnableEx(ADC0_BASE, ADC_INT_DMA_SS2);
			//在时钟启动之前不久使用
//		MAP_ADCSequenceDMAEnable(ADC0_BASE, 2);  // 先启用DMA功能													
		/*
			MAP_IntPrioritySet(INT_ADC0SS2, NVIC_PRIORITY_HIGH);  // 设置中断优先级												
			通常在main的while之前编写			
			！！！！！！！！！！！！！！！！别忘了写												
		*/
		MAP_IntPrioritySet(INT_ADC0SS2, 2);															
		MAP_IntEnable(INT_ADC0SS2);
		//使用中断要在外设级和nvic级都要enable
		//当然也要有总的开启"可屏蔽"中断的	MAP_IntMasterEnable												 
		MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);

}

void ADC_TimerStart(void)
{
		/* Reconfigure the channel control structure and enable the channel */
		MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
																		 UDMA_MODE_BASIC,
																		 (void *)&ADC0->SSFIFO2, (void *)adc_buff,
																		 1024);
		//返回变量或类型占用的字节数（Bytes）  但是我们需要的是个数而非字节数  
		//num=sizeof(adc_buff)/sizeof(uint16_t)
		MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);//传输完成后channel会自动置为stop即disable
		
		MAP_ADCSequenceDMAEnable(ADC0_BASE, 2);
		HWREG(TIMER0_BASE + TIMER_O_TAV)=0;//使用的32位模式，操作TIMER_O_TAV就够了
		MAP_TimerEnable(TIMER0_BASE, TIMER_A);
}
void ADC_TimerStop(void)
{
		MAP_ADCSequenceDMADisable(ADC0_BASE, 2);
		//时钟关闭
		MAP_TimerDisable(TIMER0_BASE, TIMER_A);
		HWREG(TIMER0_BASE + TIMER_O_TAV)=0;//使用的32位模式，操作TIMER_O_TAV就够了
}

void ADC_Change_freq(float freq)
{
    // 检查无效频率
    if (freq <= 0)
    {
        return;
    }
    uint16_t psc;
    uint16_t period;

    // 计算目标值：clock / freq = (psc + 1) * (period + 1)
    uint32_t target = round(1.0f*ui32SysClock / freq);

    // 遍历 psc，寻找合适的 period
    for (psc = 0; psc < 65535; psc++)
    {
        // 如果除数超过目标值，跳过本次循环
        if (psc+1 > target)
        {
            return;
        }
        // 计算 period + 1 = target / divisor
        uint32_t period_plus_one = target/(psc+1);
        // 检查 period 是否在 16 位范围内 (最大 65535)
        if (period_plus_one > 65536) // period_plus_one <= 65536 确保 period <= 65535
        {
            continue;
        }
        // 验证是否精确匹配
        uint32_t actual = (psc+1)*period_plus_one;
        if (actual == target)
        {
            period = period_plus_one - 1;

            // 配置 TIM3
            MAP_TimerEnable(TIMER0_BASE, TIMER_A);                          // 关闭 TIM3 以进行配置
            MAP_TimerPrescaleSet(TIMER0_BASE, TIMER_A,psc);                 // 设置自动重载值 (period)
            TimerLoadSet(TIMER0_BASE,TIMER_A,period); 											// 设置预分频器
            Fs = ui32SysClock / ((psc + 1) * (period + 1));         				// 计算并更新实际采样频率
            return;
        }
    }
    // 未找到合适的 psc 和 period，可添加错误处理
    // 示例：printf("错误：无法为频率 %u 配置合适的 psc 和 period\n", freq);
}

//buf有4096个16位
//DMA传输完成后：ADC模块（而非DMA控制器）会生成ADC_INT_DMA_SSx中断标志
//这种设计是为了让ADC模块统一管理所有中断源（包括DMA完成事件）
uint32_t DMA_Transmit_num=1;

void ADC0SS2_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Get the interrupt status from the ADC */
    getIntStatus = MAP_ADCIntStatusEx(ADC0_BASE, true);

    /* If the interrupt status for Sequencer-3 is set the
     * clear the status and read the data */
    if((getIntStatus & ADC_INT_DMA_SS2) == ADC_INT_DMA_SS2)
    {
			MAP_ADCIntClearEx(ADC0_BASE, ADC_INT_DMA_SS2);
			if(DMA_Transmit_num<4){
					/* Clear the ADC interrupt flag. */
				//操作ui32Base + ADC_O_ISC   (ADC Interrupt Status and Clear)
					
					
					/* Reconfigure the channel control structure and enable the channel */
					MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
																		 UDMA_MODE_BASIC,
																		 (void *)&ADC0->SSFIFO2, (void *)(adc_buff+(DMA_Transmit_num<<10)),
																		 1024);
	//返回变量或类型占用的字节数（Bytes）  但是我们需要的是个数而非字节数  
	//num=sizeof(adc_buff)/sizeof(uint16_t)
					MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);//传输完成后channel会自动置为stop即disable
					DMA_Transmit_num++;
			}else{
				//DMA于时钟关闭
				ADC_TimerStop();
				DMA_Transmit_num=0;
				ADC_Done=true;
			}
    }
}