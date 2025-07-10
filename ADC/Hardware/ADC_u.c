#include "ADC_u.h"

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

uint16_t adc_buff[FFT_LENGTH];
void ADC_SimpleInit(void)
{
	//modul init*****************************************
	/* Enable the clock to GPIO Port E and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)));

    /* Configure PE0 as ADC input channel */
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);

    /* Enable the clock to ADC-0 and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)));

		//Squencer Init**************************************
		/*设计Squencer中的certain step 
		Configure Sequencer 2 to sample the analog channel : AIN0. The
     * end of conversion and interrupt generation is set for AIN0 */
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH0 | 
                                 ADC_CTL_END);// 去掉ADC_CTL_IE orADC采样序列完成就会触发中断  虽然但是，用了ADC_CTL_IE好像没啥影响，但是去掉也能用
																							//  ADC_CTL_IE 和 ADC_INT_DMA_SS3，但它们的 中断服务程序是同一个（ADC0SS3_IRQHandler
																							//通常用于软件读取ADC数据 但我们用dma
    /* Enable sample sequence 2 with a Processor signal trigger.  Sequencer 2
     * will do a single sample.*/
		//关于时钟的配置见相关工程中timer0的配置 
    MAP_ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_TIMER, 2);//1个触发信号 → 执行Sequencer内所有Steps
		//Squencer大致配置完毕
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
		MAP_ADCSequenceDMAEnable(ADC0_BASE, 2);  // 先启用DMA功能													
		/*
			MAP_IntPrioritySet(INT_ADC0SS2, NVIC_PRIORITY_HIGH);  // 设置中断优先级												
			MAP__enableInterrupt();
			通常在main的while之前编写			
			！！！！！！！！！！！！！！！！别忘了写												
		*/													
		MAP_IntEnable(INT_ADC0SS2);
		//使用中断要在外设级和nvic级都要enable
		//当然也要有总的开启"可屏蔽"中断的	MAP_IntMasterEnable												 
		MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);

}
//DMA传输完成后：ADC模块（而非DMA控制器）会生成ADC_INT_DMA_SSx中断标志
//这种设计是为了让ADC模块统一管理所有中断源（包括DMA完成事件）
void ADC0SS3_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Get the interrupt status from the ADC */
    getIntStatus = MAP_ADCIntStatusEx(ADC0_BASE, true);

    /* If the interrupt status for Sequencer-3 is set the
     * clear the status and read the data */
    if((getIntStatus & ADC_INT_DMA_SS2) == ADC_INT_DMA_SS2)
    {
        /* Clear the ADC interrupt flag. */
			//操作ui32Base + ADC_O_ISC   (ADC Interrupt Status and Clear)
        MAP_ADCIntClearEx(ADC0_BASE, ADC_INT_DMA_SS2);
				
        /* Reconfigure the channel control structure and enable the channel */
        MAP_uDMAChannelTransferSet(UDMA_CH16_ADC0_2 | UDMA_PRI_SELECT,
                                   UDMA_MODE_BASIC,
                                   (void *)&ADC0->SSFIFO2, (void *)&adc_buff,
                                   sizeof(adc_buff)/2);
//返回变量或类型占用的字节数（Bytes）  但是我们需要的是个数而非字节数  
//num=sizeof(adc_buff)/sizeof(uint16_t)
				MAP_uDMAChannelEnable(UDMA_CH16_ADC0_2);//传输完成后channel会自动置为stop即disable
    }
}